#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>
#include "display.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

LV_IMAGE_DECLARE(dashboard);

#define STACKSIZE 4096
#define PRIORITY 10
/*
 * Card layout measured directly from images/dashboard.png (400x300): three
 * 111px-wide cards, each with an icon glyph ending by y=146 and its unit
 * label ("C" / "%" / "mBar") starting by y=227, leaving a shared y=146..225
 * gap for a value readout between them.
 */
#define CARD_WIDTH 111
#define CARD1_X 28
#define CARD2_X 148
#define CARD3_X 266
#define VALUE_LABEL_Y 178

const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static lv_obj_t *temperature_label;
static lv_obj_t *humidity_label;
static lv_obj_t *pressure_label;
static bool forceUpdate=false;
bool displayReady=false;

struct sensor_readings
{
  float temperature_c;
  uint32_t pressure_pa;
  float humidity_pct;
};

K_MSGQ_DEFINE(sensor_readings_msgq, sizeof(struct sensor_readings), 1, 4);

void displayTask(void *dummy1, void *dummy2, void *dummy3);

void display_update_readings(float temperature_c, uint32_t pressure_pa, float humidity_pct, bool update)
{
  struct sensor_readings readings = {temperature_c, pressure_pa, humidity_pct};

  k_msgq_purge(&sensor_readings_msgq);
  k_msgq_put(&sensor_readings_msgq, &readings, K_NO_WAIT);
  forceUpdate = update;
}

static int round_to_int(float value)
{
  return (int)(value + (value >= 0.0f ? 0.5f : -0.5f));
}

K_THREAD_DEFINE(display, STACKSIZE,
                displayTask, NULL, NULL, NULL,
                PRIORITY, 0, 0);

static lv_obj_t *create_value_label(lv_obj_t *parent, lv_coord_t x)
{
  lv_obj_t *label = lv_label_create(parent);

  lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_36, LV_PART_MAIN);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_width(label, CARD_WIDTH);
  lv_obj_set_pos(label, x, VALUE_LABEL_Y);
  lv_label_set_text(label, "--");

  return label;
}

void displayTask(void *dummy1, void *dummy2, void *dummy3)
{

  if (!device_is_ready(display_dev))
  {
    LOG_ERR("Display device not ready");
    return;
  }

  lv_obj_t *screen = lv_screen_active();

  /*
   * No theme is ever applied (no lv_display_set_theme() call anywhere),
   * so the screen keeps LVGL's library-wide default style, where bg_opa
   * is LV_OPA_TRANSP. Force it to opaque white so every render writes
   * well-defined white pixels instead of leftover/garbage ones.
   */
  lv_obj_set_style_bg_color(screen, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);

  /*
   * ssd16xx_init() leaves blanking_on = false, so the very first flush
   * below would otherwise go out through the PARTIAL profile. Explicit
   * blanking_on()/blanking_off() around the boot render forces the FULL
   * profile and a real clearing refresh - see "7. Blanking on/off no-op"
   * in Docs/SSD1683_bringup_bugs_and_fixes.md.
   */
  display_blanking_on(display_dev);

  /*
   * The very first full-screen invalidation of the program's lifetime
   * never reaches lvgl_rounder_cb_mono() and gets permanently cached
   * with the wrong, byte-rounded bounds - see "5. First invalidation
   * of the program's lifetime never reaches the rounder" in
   * Docs/SSD1683_bringup_bugs_and_fixes.md. Burn a disposable
   * invalidate+render cycle here, before anything is on screen, so
   * the real image render below is the second invalidation instead.
   */
  lv_obj_invalidate(screen);
  lv_timer_handler();

  lv_obj_t *img = lv_image_create(screen);

  lv_image_set_src(img, &dashboard);
  lv_obj_center(img);

  /*
   * Value readouts sit on top of the static background image, in the
   * gap between each card's icon and its unit label.
   */
  temperature_label = create_value_label(screen, CARD1_X);
  humidity_label = create_value_label(screen, CARD2_X);
  pressure_label = create_value_label(screen, CARD3_X);

  lv_timer_handler();

  /*
   * Turning blanking back off triggers the actual full-refresh activate
   * of everything written above, cleanly clearing the panel instead of
   * diffing against its unknown power-up RAM state.
   */
  display_blanking_off(display_dev);

  int last_temperature_c = INT32_MIN;
  int last_pressure_mbar = 0;
  int last_humidity_pct = 0;

  displayReady=true;

  while (1)
  {
    struct sensor_readings readings;

    if (k_msgq_get(&sensor_readings_msgq, &readings, K_MSEC(1000)) != 0)
      continue;

    int temperature_c = round_to_int(readings.temperature_c);
    int pressure_mbar = round_to_int((float)readings.pressure_pa / 100.0f);
    int humidity_pct = round_to_int(readings.humidity_pct);
    bool changed = false;

    if (temperature_c != last_temperature_c || forceUpdate)
    {      
      lv_label_set_text_fmt(temperature_label, "%d", temperature_c);
      last_temperature_c = temperature_c;      
      changed = true;
    }

    if (pressure_mbar != last_pressure_mbar || forceUpdate)
    {
      lv_label_set_text_fmt(pressure_label, "%d", pressure_mbar);
      last_pressure_mbar = pressure_mbar;
      changed = true;
    }

    if (humidity_pct != last_humidity_pct || forceUpdate)
    {
      lv_label_set_text_fmt(humidity_label, "%d", humidity_pct);
      last_humidity_pct = humidity_pct;
      changed = true;
    }

    if (changed)
      lv_timer_handler();
  }
}

void display_bmp280_dashes()
{
  if(displayReady)
  {
    lv_label_set_text(temperature_label, "--");
    lv_label_set_text(pressure_label, "--");  
    lv_timer_handler();
  }

}

void display_aht20_dashes()
{
  if(displayReady)
  {
    lv_label_set_text(humidity_label, "--");
    lv_timer_handler();
  }

}