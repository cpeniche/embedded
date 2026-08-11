#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

LV_IMAGE_DECLARE(dashboard);

/*
 * Card layout measured directly from images/dashboard.png (400x300): three
 * 111px-wide cards, each with an icon glyph ending by y=146 and its unit
 * label ("C" / "%" / "mBar") starting by y=227, leaving a shared y=146..225
 * gap for a value readout between them.
 */
#define CARD_WIDTH   111
#define CARD1_X      28
#define CARD2_X      148
#define CARD3_X      266
#define VALUE_LABEL_Y 178

static lv_obj_t *temperature_label;
static lv_obj_t *humidity_label;
static lv_obj_t *pressure_label;

static lv_obj_t *create_value_label(lv_obj_t *parent, lv_coord_t x)
{
	lv_obj_t *label = lv_label_create(parent);

	lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_width(label, CARD_WIDTH);
	lv_obj_set_pos(label, x, VALUE_LABEL_Y);
	lv_label_set_text(label, "--");

	return label;
}

int main(void)
{
	const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

	if (!device_is_ready(display_dev)) {
		LOG_ERR("Display device not ready");
		return 0;
	}

	lv_obj_t *screen = lv_screen_active();

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
	 * gap between each card's icon and its unit label. Placeholder text
	 * for now - replace with lv_label_set_text() calls once real sensor
	 * readings are wired in, followed by lv_obj_invalidate(screen) (or
	 * just the changed label) + lv_timer_handler() to push the update.
	 */
	temperature_label = create_value_label(screen, CARD1_X);
	humidity_label = create_value_label(screen, CARD2_X);
	pressure_label = create_value_label(screen, CARD3_X);

	lv_timer_handler();

	while (1) {
		k_msleep(1000);
	}

	return 0;
}
