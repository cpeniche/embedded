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
 * two standard formats instead and repacks incoming buffers into the
 * native encoding itself - see jd79661_write_cmd_packed():
 *   - PIXEL_FORMAT_MONO01 (8 pixels/byte, MSB first, 0=black/1=white),
 *     for black/white-only content.
 *   - PIXEL_FORMAT_I_4 (2 pixels/byte, high nibble first), for full
 *     black/white/yellow/red content. Only the low 2 bits of each
 *     nibble are meaningful; they are the native color code (see
 *     JD79661_COLOR_* in jd79661_regs.h) and are used as-is.
 *
 * Mostly the documented register set is used, plus PFS (0x03), TSE
 * (0x41), GSST (0x65) and PWS (0xE3) with the vendor reference
 * driver's values. That same driver also issues six commands (0x4D,
 * 0xB4, 0xB5, 0xE0, 0xE7, 0xE9) whose function isn't covered by the
 * datasheet at all; real panels have been observed to need them, so
 * they are reproduced here verbatim too - see the JD79661_CMD_UNDOC_*
 * comment in jd79661_regs.h. jd79661_set_profile() issues every one
 * of these in the same order as the vendor driver's initialization
 * sequence.
 *
 * As with the UC81xx driver, first gate/source should be 0 - see the
 * GSST write in jd79661_set_profile().
 */

#define JD79661_PIXELS_PER_BYTE 4U
#define JD79661_MONO_PIXELS_PER_BYTE 8U
#define JD79661_I4_PIXELS_PER_BYTE 2U

/* Single-chip hardware maximums; fallback for the optional devicetree
 * "max-width"/"max-height" properties (e.g. for a cascaded panel).
 */
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
	uint16_t max_width;
	uint16_t max_height;

	struct jd79661_dt_array softstart;

	const struct jd79661_profile *profiles[JD79661_NUM_PROFILES];
};

struct jd79661_data
{
	bool blanking_on;
	enum jd79661_profile_type profile;
	enum display_pixel_format pixel_format;

	/*
	 * Union of all windows written while blanking_on is true, used by
	 * jd79661_blanking_off() to scope the final refresh to what was
	 * actually touched instead of unconditionally refreshing the
	 * whole screen. dirty_valid is false when nothing has been
	 * written yet in the current blanking-on session.
	 */
	bool dirty_valid;
	uint16_t dirty_x1, dirty_y1, dirty_x2, dirty_y2;
};

static inline void jd79661_busy_wait(const struct device *dev)
{
	const struct jd79661_config *config = dev->config;
	int pin = gpio_pin_get_dt(&config->busy_gpio);

	while (pin == 0)
	{
		__ASSERT(pin >= 0, "Failed to get pin level");
		k_sleep(K_MSEC(JD79661_BUSY_DELAY));
		pin = gpio_pin_get_dt(&config->busy_gpio);
	}
}

/*
 * Write a single data byte (DC high) as its own, independent SPI
 * transaction - the vendor reference driver's EPD_2IN15G_SendData()
 * (Waveshare's EPD_2in15g.c) issues one SPI write per byte, giving
 * each byte its own chip-select cycle rather than bursting several
 * bytes under one CS assertion. All multi-byte data writes in this
 * driver are built out of repeated calls to this function to match
 * that behavior byte-for-byte.
 */
static inline int jd79661_write_data_byte(const struct device *dev, uint8_t byte)
{
	const struct jd79661_config *config = dev->config;
	struct display_buffer_descriptor mipi_desc = {
			.height = 1,
			.width = 1,
			.pitch = 1,
			.buf_size = 1,
	};

	return mipi_dbi_write_display(config->mipi_dev, &config->dbi_config,
																&byte, &mipi_desc, PIXEL_FORMAT_MONO01);
}

static inline int jd79661_write_cmd(const struct device *dev, uint8_t cmd,
																		const uint8_t *data, size_t len)
{
	const struct jd79661_config *config = dev->config;
	int err;

	jd79661_busy_wait(dev);

	err = mipi_dbi_command_write(config->mipi_dev, &config->dbi_config,
															 cmd, NULL, 0);
	if (err < 0)
	{
		goto out;
	}

	for (size_t i = 0; i < len; i++)
	{
		err = jd79661_write_data_byte(dev, data[i]);
		if (err < 0)
		{
			goto out;
		}
	}

out:
	mipi_dbi_release(config->mipi_dev, &config->dbi_config);
	return err;
}

/*
 * Pack 4 native 2-bit color codes (see JD79661_COLOR_* in
 * jd79661_regs.h) into a single JD79661AA data byte, MSB first,
 * matching the Pixel1..Pixel4 layout of JD79661_CMD_DTM.
 */
static inline uint8_t jd79661_pack4(uint8_t p0, uint8_t p1, uint8_t p2, uint8_t p3)
{
	return ((p0 & 0x3) << 6) | ((p1 & 0x3) << 4) | ((p2 & 0x3) << 2) | (p3 & 0x3);
}

/*
 * Expand one PIXEL_FORMAT_MONO01 byte (8 pixels, MSB first, 0=black/
 * 1=white) into two native JD79661AA bytes (4 pixels each).
 */
static inline void jd79661_pack_mono_byte(uint8_t mono, uint8_t out[2])
{
#define JD79661_GRAY(px) ((px) ? JD79661_COLOR_WHITE : JD79661_COLOR_BLACK)
	out[0] = jd79661_pack4(JD79661_GRAY((mono >> 7) & 1), JD79661_GRAY((mono >> 6) & 1),
												 JD79661_GRAY((mono >> 5) & 1), JD79661_GRAY((mono >> 4) & 1));
	out[1] = jd79661_pack4(JD79661_GRAY((mono >> 3) & 1), JD79661_GRAY((mono >> 2) & 1),
												 JD79661_GRAY((mono >> 1) & 1), JD79661_GRAY(mono & 1));
#undef JD79661_GRAY
}

/*
 * Fold two PIXEL_FORMAT_I_4 bytes (4 pixels, high nibble first; only
 * the low 2 bits of each nibble are used, as the native color code -
 * see JD79661_COLOR_* in jd79661_regs.h) into a single native
 * JD79661AA data byte.
 */
static inline uint8_t jd79661_pack_i4_bytes(uint8_t i4_0, uint8_t i4_1)
{
	return jd79661_pack4((i4_0 >> 4) & 0x3, i4_0 & 0x3, (i4_1 >> 4) & 0x3, i4_1 & 0x3);
}

static inline int jd79661_write_cmd_pattern(const struct device *dev,
																						uint8_t cmd,
																						uint8_t pattern, size_t len)
{
	const struct jd79661_config *config = dev->config;
	int err;

	jd79661_busy_wait(dev);

	err = mipi_dbi_command_write(config->mipi_dev, &config->dbi_config,
															 cmd, NULL, 0);
	if (err < 0)
	{
		return err;
	}

	while (len--)
	{
		err = jd79661_write_data_byte(dev, pattern);
		if (err < 0)
		{
			goto out;
		}
	}

out:
	mipi_dbi_release(config->mipi_dev, &config->dbi_config);
	return err;
}

/*
 * Like jd79661_write_cmd(), but the caller's buffer is a standard
 * PIXEL_FORMAT_MONO01 (8 pixels/byte) or PIXEL_FORMAT_I_4 (2
 * pixels/byte) buffer that gets repacked into the panel's native
 * 2-bit-per-pixel encoding (4 pixels/byte) as it is streamed out.
 */
static inline int jd79661_write_cmd_packed(const struct device *dev, uint8_t cmd,
																							 enum display_pixel_format pixel_format,
																							 const uint8_t *buf, size_t buf_len)
{
	const struct jd79661_config *config = dev->config;
	int err;

	jd79661_busy_wait(dev);

	err = mipi_dbi_command_write(config->mipi_dev, &config->dbi_config,
															 cmd, NULL, 0);
	if (err < 0)
	{
		return err;
	}

	if (pixel_format == PIXEL_FORMAT_I_4)
	{
		for (size_t i = 0; i < buf_len; i += 2)
		{
			err = jd79661_write_data_byte(dev, jd79661_pack_i4_bytes(buf[i], buf[i + 1]));
			if (err < 0)
			{
				goto out;
			}
		}

		goto out;
	}

	for (size_t i = 0; i < buf_len; i++)
	{
		uint8_t packed[2];

		jd79661_pack_mono_byte(buf[i], packed);

		for (size_t j = 0; j < 2; j++)
		{
			err = jd79661_write_data_byte(dev, packed[j]);
			if (err < 0)
			{
				goto out;
			}
		}
	}

out:
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
	 * The writes below follow the exact command order and values of
	 * the vendor reference driver's EPD_2IN15G_Init() (Waveshare's
	 * EPD_2in15g.c), so that this driver reproduces its init sequence
	 * command-for-command. Steps sourced from optional devicetree
	 * profile overrides are skipped if the override isn't present.
	 */

	/*
	 * Undocumented vendor init command, see JD79661_CMD_UNDOC_4D in
	 * jd79661_regs.h.
	 */
	if (jd79661_write_cmd_uint8(dev, JD79661_CMD_UNDOC_4D, JD79661_UNDOC_4D_VAL))
	{
		return -EIO;
	}

	/* Panel settings: MTP LUT, default scan/shift directions, soft reset */
	LOG_DBG("PSR: %#hhx %#hhx", psr[0], psr[1]);
	if (jd79661_write_cmd(dev, JD79661_CMD_PSR, psr, sizeof(psr)))
	{
		return -EIO;
	}

	if (p)
	{
		/* Power setting */
		LOG_HEXDUMP_DBG(p->pwr.data, p->pwr.len, "PWR");
		if (jd79661_write_array_opt(dev, JD79661_CMD_PWR, &p->pwr))
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

	if (p)
	{
		/* Booster soft start */
		if (jd79661_write_array_opt(dev, JD79661_CMD_BTST,
																&config->softstart))
		{
			return -EIO;
		}

		if (p->override_pll)
		{
			LOG_DBG("PLL: %#hhx", p->pll);
			if (jd79661_write_cmd_uint8(dev, JD79661_CMD_PLL, p->pll))
			{
				return -EIO;
			}
		}
	}

	/*
	 * Temperature sensor calibration, see JD79661_CMD_TSE in
	 * jd79661_regs.h.
	 */
	if (jd79661_write_cmd_uint8(dev, JD79661_CMD_TSE, JD79661_TSE_VAL))
	{
		return -EIO;
	}

	if (p)
	{
		if (p->override_cdi)
		{
			/*
			 * Written verbatim, matching the vendor reference
			 * driver - see the JD79661_CDI_DEFAULT comment in
			 * jd79661_regs.h.
			 */
			LOG_DBG("CDI: %#hhx", p->cdi);
			if (jd79661_write_cmd_uint8(dev, JD79661_CMD_CDI, p->cdi))
			{
				return -EIO;
			}
		}

		if (jd79661_write_array_opt(dev, JD79661_CMD_TCON, &p->tcon))
		{
			return -EIO;
		}
	}

	/* Set panel resolution */
	if (jd79661_set_tres(dev))
	{
		return -EIO;
	}

	/*
	 * Gate/source start setting, see JD79661_CMD_GSST in
	 * jd79661_regs.h.
	 */
	{
		const uint8_t gsst[4] = {
				JD79661_GSST_VAL0,
				JD79661_GSST_VAL1,
				JD79661_GSST_VAL2,
				JD79661_GSST_VAL3,
		};

		if (jd79661_write_cmd(dev, JD79661_CMD_GSST, gsst, sizeof(gsst)))
		{
			return -EIO;
		}
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
	 * VDCS has no counterpart in the vendor reference driver's init
	 * sequence; it remains a purely optional extension, applied last.
	 */
	if (p && p->override_vdcs)
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

	/* Turn on: booster, controller, regulators, and sensor.*/
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

	/* Turn off: booster, controller, regulators, and sensor.*/
	if (jd79661_write_cmd(dev, JD79661_CMD_POF, NULL, 0))
	{
		return -EIO;
	}

	return 0;
}

static int jd79661_blanking_off(const struct device *dev)
{
	const struct jd79661_config *config = dev->config;
	struct jd79661_data *data = dev->data;

	if (data->blanking_on)
	{
		/*
		 * PTL is stateful: it still holds whatever window the last
		 * jd79661_write() band left it at. Reset it before the
		 * refresh below to the union of everything actually written
		 * during this blanking-on session (tracked by jd79661_write()
		 * into data->dirty_*), or DRF would only optically update
		 * that last band's tiny window - the SRAM contents would be
		 * correct, but the rest of what was written would never
		 * actually be refreshed. Falls back to the whole screen if
		 * nothing was written (e.g. blanking was toggled on/off with
		 * no write in between).
		 */
		if (data->dirty_valid)
		{
			if (jd79661_set_ptl(dev, data->dirty_x1, data->dirty_y1,
													data->dirty_x2, data->dirty_y2, false))
			{
				return -EIO;
			}
		}
		else
		{
			if (jd79661_set_ptl(dev, 0, 0, config->width - 1, config->height - 1, false))
			{
				return -EIO;
			}
		}

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
	data->dirty_valid = false;

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
	unsigned int pixels_per_byte = (data->pixel_format == PIXEL_FORMAT_I_4) ?
											 JD79661_I4_PIXELS_PER_BYTE : JD79661_MONO_PIXELS_PER_BYTE;
	size_t buf_len;

	LOG_DBG("x %u, y %u, height %u, width %u, pitch %u",
					x, y, desc->height, desc->width, desc->pitch);

	buf_len = MIN(desc->buf_size,
								desc->height * desc->width / pixels_per_byte);
	__ASSERT(desc->width <= desc->pitch, "Pitch is smaller than width");
	__ASSERT(buf != NULL, "Buffer is not available");
	__ASSERT(buf_len != 0U, "Buffer of length zero");
	__ASSERT(!(desc->width % pixels_per_byte),
					 "Buffer width not multiple of %d", pixels_per_byte);
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

	if (data->blanking_on)
	{
		/*
		 * Track the union of every window written during this
		 * blanking-on session, so jd79661_blanking_off() can scope
		 * its final refresh to what was actually touched instead of
		 * the whole screen - see the dirty_* comment in
		 * struct jd79661_data.
		 */
		if (!data->dirty_valid)
		{
			data->dirty_x1 = x;
			data->dirty_y1 = y;
			data->dirty_x2 = x_end_idx;
			data->dirty_y2 = y_end_idx;
			data->dirty_valid = true;
		}
		else
		{
			data->dirty_x1 = MIN(data->dirty_x1, x);
			data->dirty_y1 = MIN(data->dirty_y1, y);
			data->dirty_x2 = MAX(data->dirty_x2, x_end_idx);
			data->dirty_y2 = MAX(data->dirty_y2, y_end_idx);
		}
	}

	/*
	 * The window must be addressed on every write, whether or not
	 * blanking is on: a full-frame composite is typically streamed
	 * in multiple bands (one jd79661_write() call per band, each
	 * covering a different y range), and JD79661AA ignores the
	 * window bounds while PMODE is 0 - disabling it here would make
	 * every band land at the same address instead of its own y
	 * range. blanking_on only controls whether a refresh is fired
	 * immediately below; the window is always honored.
	 */
	if (jd79661_set_ptl(dev, x, y, x_end_idx, y_end_idx, true))
	{
		return -EIO;
	}

	if (jd79661_write_cmd_packed(dev, JD79661_CMD_DTM, data->pixel_format,
																	 (const uint8_t *)buf, buf_len))
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
	struct jd79661_data *data = dev->data;

	memset(caps, 0, sizeof(struct display_capabilities));
	caps->x_resolution = config->width;
	caps->y_resolution = config->height;

	/*
	 * The JD79661AA's native format is 2 bits/pixel (4 pixels/byte,
	 * see JD79661_CMD_DTM / R10h in the datasheet), for which Zephyr's
	 * display API has no dedicated pixel format. Two standard formats
	 * are advertised instead; jd79661_write() repacks them into the
	 * panel's native encoding before sending it (see
	 * jd79661_write_cmd_packed()):
	 *   - PIXEL_FORMAT_MONO01 (8 pixels/byte) for black/white content.
	 *   - PIXEL_FORMAT_I_4 (2 pixels/byte) for full black/white/
	 *     yellow/red content, indexed by JD79661_COLOR_* (see
	 *     jd79661_regs.h).
	 */
	caps->supported_pixel_formats = PIXEL_FORMAT_MONO01 | PIXEL_FORMAT_I_4;
	caps->current_pixel_format = data->pixel_format;
	caps->screen_info = SCREEN_INFO_MONO_MSB_FIRST | SCREEN_INFO_EPD;

#if defined(CONFIG_DISPLAY_COLOR_PALETTE)
	caps->color_palette[JD79661_COLOR_BLACK] =
		(struct display_palette_color){.r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF};
	caps->color_palette[JD79661_COLOR_WHITE] =
		(struct display_palette_color){.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF};
	caps->color_palette[JD79661_COLOR_YELLOW] =
		(struct display_palette_color){.r = 0xFF, .g = 0xFF, .b = 0x00, .a = 0xFF};
	caps->color_palette[JD79661_COLOR_RED] =
		(struct display_palette_color){.r = 0xFF, .g = 0x00, .b = 0x00, .a = 0xFF};
#endif
}

static int jd79661_set_pixel_format(const struct device *dev,
																		const enum display_pixel_format pf)
{
	struct jd79661_data *data = dev->data;

	if (pf == PIXEL_FORMAT_MONO01 || pf == PIXEL_FORMAT_I_4)
	{
		data->pixel_format = pf;
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

	data->profile = JD79661_PROFILE_INVALID;
	data->pixel_format = PIXEL_FORMAT_MONO01;
	data->blanking_on = true;

	if (jd79661_set_profile(dev, JD79661_PROFILE_FULL))
	{
		return -EIO;
	}

	/*
	 * Establish a full white baseline. This is also required before
	 * any windowed (PTL) refresh: partial/windowed refreshes need a
	 * prior full refresh to behave correctly, or the update can bleed
	 * outside the intended window - see jd79661_blanking_off().
	 */
	if (jd79661_clear_and_write_buffer(dev, JD79661_COLOR_BYTE(JD79661_COLOR_WHITE), true))
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

	if (config->width > config->max_width ||
			config->height > config->max_height)
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
			.max_width = DT_PROP_OR(n, max_width, JD79661_MAX_WIDTH),                  \
			.max_height = DT_PROP_OR(n, max_height, JD79661_MAX_HEIGHT),               \
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
