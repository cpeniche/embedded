#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(display, LOG_LEVEL_DBG);
#include <lvgl.h>

#define STACKSIZE 4096
#define PRIORITY 7

void displayTask(void *dummy1, void *dummy2, void *dummy3);

K_THREAD_DEFINE(display, STACKSIZE,
                displayTask, NULL, NULL, NULL,
                PRIORITY, 0, 0);

const struct device *display_dev;

void displayTask(void *dummy1, void *dummy2, void *dummy3)
{

  display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
  if (!device_is_ready(display_dev))
  {
    LOG_ERR("Device not ready, aborting test");
  }

  lv_obj_t *screen = lv_screen_active();
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x00), 0);
  lv_obj_set_style_text_color(screen, lv_color_hex(0xffffff), 0);
  lv_obj_t *label = lv_label_create(screen);
  lv_obj_set_align(label, LV_ALIGN_CENTER);
  lv_label_set_text(label, "Hello world");

  while (1)
  {
    lv_timer_handler();
    LOG_DBG("Display Task");
    k_sleep(K_SECONDS(2));
  }
}