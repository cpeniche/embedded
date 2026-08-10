/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/**
 * @brief  The application entry point.
 * @retval int
 */

lv_obj_t *label;
lv_obj_t *screen;

int main(void)
{
  const struct device *display_dev;
  int ret;

  display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
  if (!device_is_ready(display_dev))
  {
    LOG_ERR("Display device not ready");
    return 0;
  }

  /*
   * Landscape (400x300) is handled at the devicetree level instead of
   * via LVGL's own rotation - see the width/height/rotation
   * properties on the ssd1683 node in lvgl-display-ssd1683.dtsi and
   * the comment there for why. ssd16xx_get_capabilities() reports
   * that swapped resolution directly, so LVGL sees 400x300 from the
   * start with no runtime rotation calls needed here.
   */
  screen = lv_screen_active();

  /*
   * An "empty" screen (no widgets) may not actually generate a
   * background-fill draw task under the mono theme's defaults,
   * leaving the flush buffer holding stale/garbage content that just
   * gets sent to the panel as-is instead of a clean white fill.
   * Force an explicit, guaranteed opaque white background so there's
   * always something concrete for LVGL to actually draw and flush.
   */
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_bg_color(screen, lv_color_white(), LV_PART_MAIN);

  /*
   * The very first full-screen invalidation of the whole program's
   * lifetime has been confirmed (by directly instrumenting
   * lv_inv_area() and lvgl_rounder_cb_mono()) to never actually reach
   * the rounder callback - every subsequent invalidation does. That
   * first, "silent" call still gets permanently saved into
   * disp->inv_areas[0] with the wrong, byte-rounding-widened X bounds
   * (303 instead of 299), and lv_inv_area()'s own "skip if already
   * covered" optimization then prevents ANY later, correctly-rounded
   * call from ever overwriting it - so the bad value sticks around
   * forever once it's the first one saved.
   *
   * Burn a disposable invalidate+render cycle here first. It's
   * expected to fail the driver's bounds check and do nothing
   * visible, which is fine: nothing meaningful is on screen yet, and
   * no blanking_on/off is done around it so it can't cause a visible
   * refresh either way. This makes the *next* full-screen render the
   * second invalidation of the program's lifetime, which is what
   * we've confirmed empirically does reach the rounder correctly.
   */
  lv_obj_invalidate(screen);
  lv_timer_handler();

  /*
   * Force a full blank (empty screen, nothing drawn yet) render and
   * refresh, and hold on it for a bit, so the panel's actual state
   * can be visually confirmed clean/white BEFORE the label write
   * happens. This isolates whatever the label write does from
   * whatever the boot render did.
   *
   * Back to display_blanking_on()/_off() (the FULL profile /
   * ctrl2_full path). Comparing our driver's computed "Display
   * Update Control 2" byte against Waveshare's own EPD_4in2_V2.c
   * reference driver found the real cause of the stuck-black-
   * background bug: our devicetree never set `tssv`, so
   * ssd16xx.c's load_temp gating stayed false and every refresh ran
   * without the LOAD_TEMPERATURE bit set (sending 0xD7/0xDF instead
   * of the reference driver's known-working 0xF7/0xFF for full/
   * partial), falling back to a hardcoded generic 25C waveform load
   * instead of a real sensor-selected one. Fixed by adding
   * `tssv = <0x80>;` to the ssd1683 node (lvgl-display-ssd1683.dtsi) -
   * this was never actually a 4-grayscale-LUT incompatibility.
   *
   * ssd16xx_init() explicitly leaves data->blanking_on = false, so
   * display_blanking_off() alone here would be a no-op (it only
   * refreshes if blanking_on is already true) - explicitly turn
   * blanking on first, force the (empty) screen to be considered
   * dirty, render it, then turn blanking off to actually trigger the
   * refresh.
   */
  ret = display_blanking_on(display_dev);
  if (ret < 0 && ret != -ENOSYS)
  {
    LOG_ERR("Failed to turn blanking on (error %d)", ret);
    return 0;
  }

  lv_obj_invalidate(screen);
  lv_timer_handler();

  ret = display_blanking_off(display_dev);
  if (ret < 0 && ret != -ENOSYS)
  {
    LOG_ERR("Failed to turn blanking off (error %d)", ret);
    return 0;
  }

  k_msleep(5000);

  label = lv_label_create(screen);
  lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_text(label, "Hello Zephyr!");
  lv_obj_center(label);

  lv_timer_handler();

  ret = display_blanking_off(display_dev);
  if (ret < 0 && ret != -ENOSYS)
  {
    LOG_ERR("Failed to turn blanking off (error %d)", ret);
    return 0;
  }

  /*
   * Cycle the label text every 4s using a fast partial refresh
   * normally, and every FULL_REFRESH_EVERY updates hold blanking on
   * across a whole-screen invalidate to force a real FULL-profile
   * refresh instead, resetting contrast.
   */
#define FULL_REFRESH_EVERY 4U

  static const char *const messages[] = {"Hello World", "Hello Zephyr!"};
  uint32_t update_count = 0;

  while (1)
  {
    k_msleep(4000);

    lv_label_set_text(label, messages[update_count % (sizeof(messages) / sizeof(messages[0]))]);
    lv_obj_center(label);
    update_count++;

    if ((update_count % FULL_REFRESH_EVERY) == 0U)
    {
      ret = display_blanking_on(display_dev);
      if (ret < 0 && ret != -ENOSYS)
      {
        LOG_ERR("Failed to turn blanking on (error %d)", ret);
      }

      lv_obj_invalidate(screen);
      lv_timer_handler();

      ret = display_blanking_off(display_dev);
      if (ret < 0 && ret != -ENOSYS)
      {
        LOG_ERR("Failed to turn blanking off (error %d)", ret);
      }
    }
    else
    {
      lv_timer_handler();
    }
  }
  /* USER CODE END 3 */
}
