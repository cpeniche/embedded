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
int main(void)
{
  const struct device *display_dev;
  lv_obj_t *label;
  int ret;

  display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
  if (!device_is_ready(display_dev)) {
    LOG_ERR("Display device not ready");
    return 0;
  }

  ret = display_blanking_on(display_dev);
  if (ret < 0 && ret != -ENOSYS) {
    LOG_ERR("Failed to turn blanking on (error %d)", ret);
    return 0;
  }

  label = lv_label_create(lv_screen_active());
  lv_label_set_text(label, "Hello world!");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  lv_timer_handler();

  ret = display_blanking_off(display_dev);
  if (ret < 0 && ret != -ENOSYS) {
    LOG_ERR("Failed to turn blanking off (error %d)", ret);
    return 0;
  }

  while (1)
  {
    lv_timer_handler();
    k_msleep(100);
  }
  /* USER CODE END 3 */
}
