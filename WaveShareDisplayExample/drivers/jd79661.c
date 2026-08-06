/*
 * Copyright (c) 2025 Cactus Engineering S.L
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/mipi_dbi.h>

#include "jd79661_regs.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(jd79661, CONFIG_DISPLAY_LOG_LEVEL);

/**
 * Jadard JD79661AA compatible EPD controller driver.
 *
 * The JD79661AA is a single-chip all-in-one driver/TCON for 4-level
 * color (e.g. black/white/yellow/red) EPD panels, 2 bits per pixel,
 * up to 176x296 native resolution. Unlike the UC81xx family this
 * driver was derived from, the JD79661AA has a single frame buffer
 * (no separate "old"/"new" data planes to keep in sync) and toggles
 * partial refresh purely through the PMODE bit of the R83h (PTL)
 * command, so there is no PTIN/PTOUT command pair.
 *
 * LUTs are only ever loaded from on-chip MTP; the datasheet does not
 * document a way to upload a LUT over SPI at runtime, so (unlike the
 * UC81xx driver) profiles cannot override them.
 *
 * Zephyr's display API has no pixel format for the panel's native
 * 2-bit-per-pixel (4 pixels/byte) encoding, so this driver advertises
 * the standard 1-bit-per-pixel PIXEL_FORMAT_MONO10 (8 pixels/byte,
 * MSB first, 1=black/0=white) instead and repacks incoming buffers
 * from 8 to 4 pixels/byte itself - see jd79661_write_cmd_packed().
 *
 * Mostly the documented register set is used, plus PFS (0x03) and PWS
 * (0xE3) with the vendor reference driver's values. That same driver
 * also issues six commands (0x4D, 0xB4, 0xB5, 0xE0, 0xE7, 0xE9) whose
 * function isn't covered by the datasheet at all; real panels have
 * been observed to need them, so they are reproduced here verbatim
 * too - see the JD79661_CMD_UNDOC_* comment in jd79661_regs.h. This
 * covers every write in the vendor driver's initialization sequence.
 *
 * As with the UC81xx driver, first gate/source should be 0.
 */

#define JD79661_PIXELS_PER_BYTE 4U
#define JD79661_MONO_PIXELS_PER_BYTE 8U

#define JD79661_MAX_WIDTH 176U
#define JD79661_MAX_HEIGHT 296U

struct jd79661_dt_array
{
	uint8_t *data;
	uint8_t len;
};

enum jd79661_profile_type
{
	JD79661_PROFILE_FULL = 0,
	JD79661_PROFILE_PARTIAL,
	JD79661_NUM_PROFILES,
	JD79661_PROFILE_INVALID = JD79661_NUM_PROFILES,
};

struct jd79661_profile
{
	struct jd79661_dt_array pwr;
	struct jd79661_dt_array tcon; /* R60h: S2G, G2S */

	uint8_t cdi;
	bool override_cdi;
	uint8_t pll;
	bool override_pll;
	uint8_t vdcs;
	bool override_vdcs;
};

struct jd79661_config
{
	const struct device *mipi_dev;
	const struct mipi_dbi_config dbi_config;
	struct gpio_dt_spec busy_gpio;

	uint16_t height;
	uint16_t width;

	struct jd79661_dt_array softstart;

	const struct jd79661_profile *profiles[JD79661_NUM_PROFILES];
};

struct jd79661_data
{
	bool blanking_on;
	enum jd79661_profile_type profile;
};

static inline void jd79661_busy_wait(const struct device *dev)
{
	const struct jd79661_config *config = dev->config;
	int pin = gpio_pin_get_dt(&config->busy_gpio);

	while (pin > 0)
	{
		__ASSERT(pin >= 0, "Failed to get pin level");
		k_sleep(K_MSEC(JD79661_BUSY_DELAY));
		pin = gpio_pin_get_dt(&config->busy_gpio);
	}
}

static inline int jd79661_write_cmd(const struct device *dev, uint8_t cmd,
																		const uint8_t *data, size_t len)
{
	const struct jd79661_config *config = dev->config;
	int err;

	jd79661_busy_wait(dev);

	err = mipi_dbi_command_write(config->mipi_dev, &config->dbi_config,
															 cmd, data, len);
	mipi_dbi_release(config->mipi_dev, &config->dbi_config);
	return err;
}

static inline int jd79661_write_cmd_pattern(const struct device *dev,
																						uint8_t cmd,
																						uint8_t pattern, size_t len)
{
	const struct jd79661_config *config = dev->config;
	struct display_buffer_descriptor mipi_desc;
	int err;
	uint8_t data[64];

	jd79661_busy_wait(dev);

	err = mipi_dbi_command_write(config->mipi_dev, &config->dbi_config,
															 cmd, NULL, 0);
	if (err < 0)
	{
		return err;
	}

	/*
	 * MIPI display write API requires a display buffer descriptor.
	 * Create one that describes the buffer we are writing
	 */
	mipi_desc.height = 1;

	memset(data, pattern, sizeof(data));
	while (len)
	{
		mipi_desc.buf_size = mipi_desc.width = mipi_desc.pitch =
				MIN(len, sizeof(data));

		err = mipi_dbi_write_display(config->mipi_dev,
																 &config->dbi_config,
																 data, &mipi_desc,
																 PIXEL_FORMAT_MONO10);
		if (err < 0)
		{
			goto out;
		}

		len -= mipi_desc.buf_size;
	}

out:
	mipi_dbi_release(config->mipi_dev, &config->dbi_config);
	return err;
}

/*
 * Pack 4 monochrome pixels (1 = black, 0 = white) into a single
 * JD79661AA data byte: 4 x 2-bit gray levels, MSB first, matching the
 * Pixel1..Pixel4 layout of JD79661_CMD_DTM. Black maps to Gray0 and
 * white to Gray3, the usual ordering for this family's default LUTs.
 */
static inline uint8_t jd79661_pack4(uint8_t p0, uint8_t p1, uint8_t p2, uint8_t p3)
{
#define JD79661_GRAY(px) ((px) ? 0x0U : 0x3U)
	return (JD79661_GRAY(p0) << 6) | (JD79661_GRAY(p1) << 4) |
				 (JD79661_GRAY(p2) << 2) | JD79661_GRAY(p3);
#undef JD79661_GRAY
}

/*
 * Expand one PIXEL_FORMAT_MONO10 byte (8 pixels, MSB first) into two
 * native JD79661AA bytes (4 pixels each).
 */
static inline void jd79661_pack_mono_byte(uint8_t mono, uint8_t out[2])
{
	out[0] = jd79661_pack4((mono >> 7) & 1, (mono >> 6) & 1,
												 (mono >> 5) & 1, (mono >> 4) & 1);
	out[1] = jd79661_pack4((mono >> 3) & 1, (mono >> 2) & 1,
												 (mono >> 1) & 1, mono & 1);
}

/*
 * Like jd79661_write_cmd(), but the caller's buffer is a standard
 * PIXEL_FORMAT_MONO10 buffer (8 pixels/byte) that gets repacked into
 * the panel's native 2-bit-per-pixel encoding (4 pixels/byte) as it
 * is streamed out, chunk by chunk, to avoid needing a full-frame
 * scratch buffer.
 */
static inline int jd79661_write_cmd_packed(const struct device *dev, uint8_t cmd,
																					 const uint8_t *mono_buf, size_t mono_len)
{
	const struct jd79661_config *config = dev->config;
	struct display_buffer_descriptor mipi_desc;
	int err;
	uint8_t out[64];

	jd79661_busy_wait(dev);

	err = mipi_dbi_command_write(config->mipi_dev, &config->dbi_config,
															 cmd, NULL, 0);
	if (err < 0)
	{
		return err;
	}

	mipi_desc.height = 1;

	while (mono_len)
	{
		size_t chunk = MIN(mono_len, sizeof(out) / 2);

		for (size_t i = 0; i < chunk; i++)
		{
			jd79661_pack_mono_byte(mono_buf[i], &out[i * 2]);
		}

		mipi_desc.buf_size = mipi_desc.width = mipi_desc.pitch = chunk * 2;

		err = mipi_dbi_write_display(config->mipi_dev,
																 &config->dbi_config,
																 out, &mipi_desc,
																 PIXEL_FORMAT_MONO10);
		if (err < 0)
		{
			goto release;
		}

		mono_buf += chunk;
		mono_len -= chunk;
	}

release:
	mipi_dbi_release(config->mipi_dev, &config->dbi_config);
	return err;
}

static inline int jd79661_write_cmd_uint8(const struct device *dev, uint8_t cmd,
																					uint8_t data)
{
	return jd79661_write_cmd(dev, cmd, &data, 1);
}

static inline int jd79661_write_array_opt(const struct device *dev, uint8_t cmd,
																					const struct jd79661_dt_array *array)
{
	if (array->len && array->data)
	{
		return jd79661_write_cmd(dev, cmd, array->data, array->len);
	}
	else
	{
		return 0;
	}
}

static int jd79661_have_profile(const struct device *dev,
																enum jd79661_profile_type type)
{
	const struct jd79661_config *config = dev->config;

	return type < JD79661_NUM_PROFILES &&
				 config->profiles[type];
}

static int jd79661_set_tres(const struct device *dev)
{
	const struct jd79661_config *config = dev->config;
	const struct jd79661_tres tres = {
			.hres_hi = (config->width >> 8) & 0x03,
			.hres_lo = config->width & 0xFC,
			.vres_hi = (config->height >> 8) & 0x03,
			.vres_lo = config->height & 0xFF,
	};

	LOG_HEXDUMP_DBG(&tres, sizeof(tres), "TRES");

	return jd79661_write_cmd(dev, JD79661_CMD_TRES, (const void *)&tres, sizeof(tres));
}

static int jd79661_set_ptl(const struct device *dev, uint16_t x, uint16_t y,
													 uint16_t x_end_idx, uint16_t y_end_idx, bool enable)
{
	const struct jd79661_ptl ptl = {
			.hrst_hi = (x >> 8) & 0x03,
			.hrst_lo = x & 0xFC,
			.hred_hi = (x_end_idx >> 8) & 0x03,
			.hred_lo = x_end_idx & 0xFC,
			.vrst_hi = (y >> 8) & 0x03,
			.vrst_lo = y & 0xFF,
			.vred_hi = (y_end_idx >> 8) & 0x03,
			.vred_lo = y_end_idx & 0xFF,
			.pmode = enable ? JD79661_PTL_PMODE_EN : 0,
	};

	LOG_HEXDUMP_DBG(&ptl, sizeof(ptl), "PTL");

	return jd79661_write_cmd(dev, JD79661_CMD_PTL, (const void *)&ptl, sizeof(ptl));
}

static int jd79661_set_profile(const struct device *dev,
															 enum jd79661_profile_type type)
{
	const struct jd79661_config *config = dev->config;
	const struct jd79661_profile *p;
	struct jd79661_data *data = dev->data;
	const uint8_t psr[2] = {JD79661_PSR1_DEFAULT, JD79661_PSR2_DEFAULT};

	if (type >= JD79661_NUM_PROFILES)
	{
		return -EINVAL;
	}

	/* No need to update the current profile, so do nothing */
	if (data->profile == type)
	{
		return 0;
	}

	p = config->profiles[type];
	data->profile = type;

	LOG_DBG("Initialize JD79661 controller with profile %d", type);

	/*
	 * Undocumented vendor init command, see JD79661_CMD_UNDOC_4D in
	 * jd79661_regs.h.
	 */
	if (jd79661_write_cmd_uint8(dev, JD79661_CMD_UNDOC_4D, JD79661_UNDOC_4D_VAL))
	{
		return -EIO;
	}

	if (p)
	{
		LOG_HEXDUMP_DBG(p->pwr.data, p->pwr.len, "PWR");
		if (jd79661_write_array_opt(dev, JD79661_CMD_PWR, &p->pwr))
		{
			return -EIO;
		}

		if (jd79661_write_array_opt(dev, JD79661_CMD_BTST,
																&config->softstart))
		{
			return -EIO;
		}
	}

	/* Power-off sequence setting, see JD79661_CMD_PFS in jd79661_regs.h. */
	{
		const uint8_t pfs[3] = {
				JD79661_PFS_VAL0,
				JD79661_PFS_VAL1,
				JD79661_PFS_VAL2,
		};

		if (jd79661_write_cmd(dev, JD79661_CMD_PFS, pfs, sizeof(pfs)))
		{
			return -EIO;
		}
	}

	/* Panel settings: MTP LUT, default scan/shift directions, soft reset */
	LOG_DBG("PSR: %#hhx %#hhx", psr[0], psr[1]);
	if (jd79661_write_cmd(dev, JD79661_CMD_PSR, psr, sizeof(psr)))
	{
		return -EIO;
	}

	/* Set panel resolution */
	if (jd79661_set_tres(dev))
	{
		return -EIO;
	}

	/* Undocumented vendor init command, see JD79661_CMD_UNDOC_E7 in jd79661_regs.h. */
	if (jd79661_write_cmd_uint8(dev, JD79661_CMD_UNDOC_E7, JD79661_UNDOC_E7_VAL))
	{
		return -EIO;
	}

	/* Power saving, see JD79661_CMD_PWS in jd79661_regs.h. */
	if (jd79661_write_cmd_uint8(dev, JD79661_CMD_PWS, JD79661_PWS_DEFAULT))
	{
		return -EIO;
	}

	/*
	 * Undocumented vendor init commands, see JD79661_CMD_UNDOC_E0 /
	 * _B4 / _B5 / _E9 in jd79661_regs.h.
	 */
	if (jd79661_write_cmd_uint8(dev, JD79661_CMD_UNDOC_E0, JD79661_UNDOC_E0_VAL))
	{
		return -EIO;
	}

	if (jd79661_write_cmd_uint8(dev, JD79661_CMD_UNDOC_B4, JD79661_UNDOC_B4_VAL))
	{
		return -EIO;
	}

	if (jd79661_write_cmd_uint8(dev, JD79661_CMD_UNDOC_B5, JD79661_UNDOC_B5_VAL))
	{
		return -EIO;
	}

	if (jd79661_write_cmd_uint8(dev, JD79661_CMD_UNDOC_E9, JD79661_UNDOC_E9_VAL))
	{
		return -EIO;
	}

	/*
	 * The rest of the configuration is optional and depends on
	 * having profile overrides specified in the device tree.
	 */
	if (!p)
	{
		return 0;
	}

	if (jd79661_write_array_opt(dev, JD79661_CMD_TCON, &p->tcon))
	{
		return -EIO;
	}

	if (p->override_cdi)
	{
		/* Keep the default VBD/DDX bits, override only the interval */
		uint8_t cdi = (JD79661_CDI_DEFAULT & ~JD79661_CDI_CDI_MASK) |
									(p->cdi & JD79661_CDI_CDI_MASK);

		LOG_DBG("CDI: %#hhx", cdi);
		if (jd79661_write_cmd_uint8(dev, JD79661_CMD_CDI, cdi))
		{
			return -EIO;
		}
	}

	if (p->override_pll)
	{
		LOG_DBG("PLL: %#hhx", p->pll);
		if (jd79661_write_cmd_uint8(dev, JD79661_CMD_PLL, p->pll))
		{
			return -EIO;
		}
	}

	if (p->override_vdcs)
	{
		LOG_DBG("VDCS: %#hhx", p->vdcs);
		if (jd79661_write_cmd_uint8(dev, JD79661_CMD_VDCS, p->vdcs))
		{
			return -EIO;
		}
	}

	return 0;
}

static int jd79661_update_display(const struct device *dev)
{
	LOG_DBG("Trigger update sequence");

	/* Turn on: booster, controller, regulators, and sensor. */
	if (jd79661_write_cmd(dev, JD79661_CMD_PON, NULL, 0))
	{
		return -EIO;
	}

	k_sleep(K_MSEC(JD79661_PON_DELAY));

	if (jd79661_write_cmd_uint8(dev, JD79661_CMD_DRF, 0x00))
	{
		return -EIO;
	}

	k_sleep(K_MSEC(JD79661_BUSY_DELAY));

	/* Turn off: booster, controller, regulators, and sensor. */
	if (jd79661_write_cmd(dev, JD79661_CMD_POF, NULL, 0))
	{
		return -EIO;
	}

	return 0;
}

static int jd79661_blanking_off(const struct device *dev)
{
	struct jd79661_data *data = dev->data;

	if (data->blanking_on)
	{
		/* Update EPD panel in normal mode */
		if (jd79661_update_display(dev))
		{
			return -EIO;
		}
	}

	data->blanking_on = false;

	return 0;
}

static int jd79661_blanking_on(const struct device *dev)
{
	struct jd79661_data *data = dev->data;

	if (!data->blanking_on)
	{
		if (jd79661_set_profile(dev, JD79661_PROFILE_FULL))
		{
			return -EIO;
		}
	}

	data->blanking_on = true;

	return 0;
}

static int jd79661_write(const struct device *dev, const uint16_t x, const uint16_t y,
												 const struct display_buffer_descriptor *desc,
												 const void *buf)
{
	const struct jd79661_config *config = dev->config;
	struct jd79661_data *data = dev->data;

	uint16_t x_end_idx = x + desc->width - 1;
	uint16_t y_end_idx = y + desc->height - 1;
	size_t buf_len;

	LOG_DBG("x %u, y %u, height %u, width %u, pitch %u",
					x, y, desc->height, desc->width, desc->pitch);

	buf_len = MIN(desc->buf_size,
								desc->height * desc->width / JD79661_MONO_PIXELS_PER_BYTE);
	__ASSERT(desc->width <= desc->pitch, "Pitch is smaller than width");
	__ASSERT(buf != NULL, "Buffer is not available");
	__ASSERT(buf_len != 0U, "Buffer of length zero");
	__ASSERT(!(desc->width % JD79661_MONO_PIXELS_PER_BYTE),
					 "Buffer width not multiple of %d", JD79661_MONO_PIXELS_PER_BYTE);
	__ASSERT(!(x % JD79661_PIXELS_PER_BYTE) && !((x_end_idx + 1) % JD79661_PIXELS_PER_BYTE),
					 "Horizontal window not aligned to %d pixels", JD79661_PIXELS_PER_BYTE);

	if ((y_end_idx > (config->height - 1)) ||
			(x_end_idx > (config->width - 1)))
	{
		LOG_ERR("Position out of bounds");
		return -EINVAL;
	}

	if (!data->blanking_on)
	{
		/* Blanking isn't on, so this is a partial
		 * refresh. Request the partial profile if it
		 * exists. If a partial profile hasn't been provided,
		 * we continue to use the full refresh profile. Note
		 * that the controller still only scans a partial
		 * window.
		 *
		 * This operation becomes a no-op if the profile is
		 * already active
		 */
		if (jd79661_have_profile(dev, JD79661_PROFILE_PARTIAL) &&
				jd79661_set_profile(dev, JD79661_PROFILE_PARTIAL))
		{
			return -EIO;
		}
	}

	/*
	 * Partial mode is only enabled for on-demand writes performed
	 * while blanking is off. Writes performed while compositing a
	 * full frame (blanking on) disable it, since JD79661AA ignores
	 * the window bounds while PMODE is 0.
	 */
	if (jd79661_set_ptl(dev, x, y, x_end_idx, y_end_idx, !data->blanking_on))
	{
		return -EIO;
	}

	if (jd79661_write_cmd_packed(dev, JD79661_CMD_DTM, (const uint8_t *)buf, buf_len))
	{
		return -EIO;
	}

	/* Only refresh immediately for on-demand (partial) writes; a
	 * full-frame write is refreshed later, in blanking_off().
	 */
	if (!data->blanking_on)
	{
		if (jd79661_update_display(dev))
		{
			return -EIO;
		}
	}

	return 0;
}

static void jd79661_get_capabilities(const struct device *dev,
																		 struct display_capabilities *caps)
{
	const struct jd79661_config *config = dev->config;

	memset(caps, 0, sizeof(struct display_capabilities));
	caps->x_resolution = config->width;
	caps->y_resolution = config->height;

	/*
	 * The JD79661AA's native format is 2 bits/pixel (4 pixels/byte,
	 * see JD79661_CMD_DTM / R10h in the datasheet), for which Zephyr's
	 * display API has no dedicated pixel format. Standard 8
	 * pixels/byte PIXEL_FORMAT_MONO10 is advertised instead;
	 * jd79661_write() repacks it into the panel's native encoding
	 * before sending it (see jd79661_write_cmd_packed()).
	 */
	caps->supported_pixel_formats = PIXEL_FORMAT_MONO10;
	caps->current_pixel_format = PIXEL_FORMAT_MONO10;
	caps->screen_info = SCREEN_INFO_MONO_MSB_FIRST | SCREEN_INFO_EPD;
}

static int jd79661_set_pixel_format(const struct device *dev,
																		const enum display_pixel_format pf)
{
	if (pf == PIXEL_FORMAT_MONO10)
	{
		return 0;
	}

	LOG_ERR("not supported");
	return -ENOTSUP;
}

static int jd79661_clear_and_write_buffer(const struct device *dev,
																					uint8_t pattern, bool update)
{
	const struct jd79661_config *config = dev->config;
	const int size = config->width * config->height / JD79661_PIXELS_PER_BYTE;

	if (jd79661_write_cmd_pattern(dev, JD79661_CMD_DTM, pattern, size))
	{
		return -EIO;
	}

	if (update == true)
	{
		if (jd79661_update_display(dev))
		{
			return -EIO;
		}
	}

	return 0;
}

static int jd79661_controller_init(const struct device *dev)
{
	const struct jd79661_config *config = dev->config;
	struct jd79661_data *data = dev->data;

	if (mipi_dbi_reset(config->mipi_dev, JD79661_RESET_DELAY) < 0)
	{
		return -EIO;
	}

	k_sleep(K_MSEC(JD79661_RESET_DELAY));
	jd79661_busy_wait(dev);

	data->blanking_on = true;
	data->profile = JD79661_PROFILE_INVALID;

	if (jd79661_set_profile(dev, JD79661_PROFILE_FULL))
	{
		return -EIO;
	}

	if (jd79661_clear_and_write_buffer(dev, 0xff, false))
	{
		return -EIO;
	}

	return 0;
}

static int jd79661_init(const struct device *dev)
{
	const struct jd79661_config *config = dev->config;

	LOG_DBG("");

	if (!device_is_ready(config->mipi_dev))
	{
		LOG_ERR("MIPI device not ready");
		return -ENODEV;
	}

	if (!gpio_is_ready_dt(&config->busy_gpio))
	{
		LOG_ERR("Busy GPIO device not ready");
		return -ENODEV;
	}

	gpio_pin_configure_dt(&config->busy_gpio, GPIO_INPUT);

	if (config->width > JD79661_MAX_WIDTH ||
			config->height > JD79661_MAX_HEIGHT)
	{
		LOG_ERR("Display size out of range.");
		return -EINVAL;
	}

	return jd79661_controller_init(dev);
}

static DEVICE_API(display, jd79661_driver_api) = {
		.blanking_on = jd79661_blanking_on,
		.blanking_off = jd79661_blanking_off,
		.write = jd79661_write,
		.get_capabilities = jd79661_get_capabilities,
		.set_pixel_format = jd79661_set_pixel_format,
};

#define JD79661_MAKE_ARRAY_OPT(n, p) \
	static uint8_t data_##n##_##p[] = DT_PROP_OR(n, p, {})

#define JD79661_ASSIGN_ARRAY(n, p)   \
	{                                  \
			.data = data_##n##_##p,        \
			.len = sizeof(data_##n##_##p), \
	}

#define JD79661_PROFILE(n)                                    \
	JD79661_MAKE_ARRAY_OPT(n, pwr);                             \
	JD79661_MAKE_ARRAY_OPT(n, tcon);                            \
                                                              \
	static const struct jd79661_profile jd79661_profile_##n = { \
			.pwr = JD79661_ASSIGN_ARRAY(n, pwr),                    \
			.tcon = JD79661_ASSIGN_ARRAY(n, tcon),                  \
			.cdi = DT_PROP_OR(n, cdi, 0),                           \
			.override_cdi = DT_NODE_HAS_PROP(n, cdi),               \
			.pll = DT_PROP_OR(n, pll, 0),                           \
			.override_pll = DT_NODE_HAS_PROP(n, pll),               \
			.vdcs = DT_PROP_OR(n, vdcs, 0),                         \
			.override_vdcs = DT_NODE_HAS_PROP(n, vdcs),             \
	};

#define _JD79661_PROFILE_PTR(n) &jd79661_profile_##n

#define JD79661_PROFILE_PTR(n)           \
	COND_CODE_1(DT_NODE_EXISTS(n),         \
							(_JD79661_PROFILE_PTR(n)), \
							NULL)

#define JD79661_DEFINE(n)                                                        \
	JD79661_MAKE_ARRAY_OPT(n, softstart);                                          \
                                                                                 \
	DT_FOREACH_CHILD(n, JD79661_PROFILE);                                          \
                                                                                 \
	static const struct jd79661_config jd79661_cfg_##n = {                         \
			.mipi_dev = DEVICE_DT_GET(DT_PARENT(n)),                                   \
			.dbi_config = {                                                            \
					.mode = MIPI_DBI_MODE_SPI_4WIRE,                                       \
					.config = MIPI_DBI_SPI_CONFIG_DT(n,                                    \
																					 SPI_OP_MODE_MASTER |                  \
																							 SPI_LOCK_ON | SPI_WORD_SET(8),    \
																					 0),                                   \
			},                                                                         \
			.busy_gpio = GPIO_DT_SPEC_GET(n, busy_gpios),                              \
                                                                                 \
			.height = DT_PROP(n, height),                                              \
			.width = DT_PROP(n, width),                                                \
                                                                                 \
			.softstart = JD79661_ASSIGN_ARRAY(n, softstart),                           \
                                                                                 \
			.profiles = {                                                              \
					[JD79661_PROFILE_FULL] = JD79661_PROFILE_PTR(DT_CHILD(n, full)),       \
					[JD79661_PROFILE_PARTIAL] = JD79661_PROFILE_PTR(DT_CHILD(n, partial)), \
			},                                                                         \
	};                                                                             \
                                                                                 \
	static struct jd79661_data jd79661_data_##n = {};                              \
                                                                                 \
	DEVICE_DT_DEFINE(n, jd79661_init, NULL,                                        \
									 &jd79661_data_##n,                                            \
									 &jd79661_cfg_##n,                                             \
									 POST_KERNEL,                                                  \
									 CONFIG_DISPLAY_INIT_PRIORITY,                                 \
									 &jd79661_driver_api);

DT_FOREACH_STATUS_OKAY(jadard_jd79661aa, JD79661_DEFINE);
