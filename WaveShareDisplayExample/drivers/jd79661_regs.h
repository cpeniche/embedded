/*
 * Copyright (c) 2025 Cactus Engineering S.L
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Register definitions for the Jadard JD79661AA all-in-one EPD
 * driver/TCON, taken from "JD79661AA Data Sheet", version 1.0.4,
 * 2023/07/20.
 */

#ifndef ZEPHYR_DRIVERS_DISPLAY_JD79661_REGS_H_
#define ZEPHYR_DRIVERS_DISPLAY_JD79661_REGS_H_

/* Commands, see section 8.1 "Register Table" */
#define JD79661_CMD_PSR			0x00	/* Panel setting */
#define JD79661_CMD_PWR			0x01	/* Power setting */
#define JD79661_CMD_POF				0x02	/* Power OFF */
#define JD79661_CMD_PON				0x04	/* Power ON */
#define JD79661_CMD_BTST			0x06	/* Booster soft start */
#define JD79661_CMD_DSLP			0x07	/* Deep sleep */
#define JD79661_CMD_DTM				0x10	/* Data start transmission */
#define JD79661_CMD_DSP				0x11	/* Data stop */
#define JD79661_CMD_DRF				0x12	/* Display refresh */
#define JD79661_CMD_AUTO			0x17	/* Auto sequence */
#define JD79661_CMD_PLL				0x30	/* PLL control */
#define JD79661_CMD_TSC				0x40	/* Temperature sensor command */
#define JD79661_CMD_TSE				0x41	/* Temperature sensor calibration */
#define JD79661_CMD_TSW				0x42	/* Temperature sensor write */
#define JD79661_CMD_TSR				0x43	/* Temperature sensor read */
#define JD79661_CMD_CDI				0x50	/* VCOM and data interval setting */
#define JD79661_CMD_LPD				0x51	/* Low power detection */
#define JD79661_CMD_TCON			0x60	/* Gate/source non-overlap (S2G/G2S) */
#define JD79661_CMD_TRES			0x61	/* Resolution setting */
#define JD79661_CMD_GSST			0x65	/* Gate/source start setting */
#define JD79661_CMD_REV				0x70	/* Revision */
#define JD79661_CMD_AMV				0x80	/* Auto measure VCOM */
#define JD79661_CMD_VV				0x81	/* VCOM value */
#define JD79661_CMD_VDCS			0x82	/* VCOM DC setting */
#define JD79661_CMD_PTL				0x83	/* Partial window */
#define JD79661_CMD_PGM				0x90	/* Program mode */
#define JD79661_CMD_APG				0x91	/* Active program */
#define JD79661_CMD_RMTP			0x92	/* Read MTP data */
#define JD79661_CMD_RMRB			0x9F	/* Read MTP reserved bytes */
#define JD79661_CMD_PWS				0xE3	/* Power saving */
#define JD79661_CMD_LVSEL			0xE4	/* LVD voltage select */

/*
 * R00h (PSR) - 1st parameter
 */
#define JD79661_PSR_RST_N			BIT(0)
#define JD79661_PSR_SHD_N			BIT(1)
#define JD79661_PSR_SHL				BIT(2)
#define JD79661_PSR_UD				BIT(3)
#define JD79661_PSR_PST_MODE			BIT(5)
#define JD79661_PSR_RES_MASK			GENMASK(7, 6)
#define JD79661_PSR_RES_176X296			(0U << 6)
#define JD79661_PSR_RES_128X296			(1U << 6)
#define JD79661_PSR_RES_128X250			(2U << 6)
#define JD79661_PSR_RES_112X204			(3U << 6)

/* Reset default: RES=00b, UD=1, SHL=1, SHD_N=1, RST_N=1 */
#define JD79661_PSR1_DEFAULT \
	(JD79661_PSR_UD | JD79661_PSR_SHL | JD79661_PSR_SHD_N | JD79661_PSR_RST_N)

/*
 * R00h (PSR) - 2nd parameter
 */
#define JD79661_PSR_VC_LUTZ			BIT(0)
#define JD79661_PSR_NORG			BIT(1)
#define JD79661_PSR_TIEG			BIT(2)
#define JD79661_PSR_TS_AUTO			BIT(3)
#define JD79661_PSR_VCMZ			BIT(4)
#define JD79661_PSR_FOPT			BIT(5)
#define JD79661_PSR_LUT_EN			BIT(7)

/*
 * VC_LUTZ=1, TS_AUTO=1, FOPT=1, LUT from MTP. Matches the vendor
 * reference driver's R00h 2nd byte (0x29); FOPT is not covered by the
 * documented reset default (0x09) but has been observed necessary on
 * real panels.
 */
#define JD79661_PSR2_DEFAULT (JD79661_PSR_VC_LUTZ | JD79661_PSR_TS_AUTO | JD79661_PSR_FOPT)

/*
 * R01h (PWR) - 1st parameter
 */
#define JD79661_PWR_VDG_EN			BIT(0)
#define JD79661_PWR_VDS_EN			BIT(1)
#define JD79661_PWR_VSC_EN			BIT(2)

/*
 * R01h (PWR) - 2nd parameter
 */
#define JD79661_PWR_VGPN_MASK			GENMASK(1, 0)
#define JD79661_PWR_VGPN_20V			0x00
#define JD79661_PWR_VGPN_17V			0x01
#define JD79661_PWR_VGPN_15V			0x02
#define JD79661_PWR_VGPN_10V			0x03

/*
 * R50h (CDI) - VCOM and data interval
 */
#define JD79661_CDI_CDI_MASK			GENMASK(3, 0)
#define JD79661_CDI_DDX				BIT(4)
#define JD79661_CDI_VBD_MASK			GENMASK(7, 5)
#define JD79661_CDI_VBD_SHIFT			5

/*
 * Reset default: 10 hsync, DDX=1, VBD=Gray0. Informational only - the
 * "cdi" profile override (see jd79661_set_profile()) writes the
 * devicetree value verbatim, same as the vendor reference driver,
 * rather than merging it with this default.
 */
#define JD79661_CDI_DEFAULT			0x97

/*
 * RE3h (PWS) - power saving. VCOM_W[3:0] is a VCOM power-saving
 * width in line periods, SD_W[3:0] a source power-saving width in
 * units of 500ns (see section 8.2.31). Reset default is 0x00; the
 * value below (VCOM_W=2, SD_W=2) is what the vendor reference driver
 * (Waveshare's EPD_2in15g.c) uses.
 */
#define JD79661_PWS_SD_W_MASK			GENMASK(3, 0)
#define JD79661_PWS_VCOM_W_MASK		GENMASK(7, 4)
#define JD79661_PWS_VCOM_W_SHIFT		4

#define JD79661_PWS_DEFAULT			0x22

/*
 * R07h (DSLP) check code, must accompany the command for it to take
 * effect.
 */
#define JD79661_DSLP_CHECK_CODE			0xA5

/*
 * R17h (AUTO) sequence codes
 */
#define JD79661_AUTO_PON_DRF_POF		0xA5
#define JD79661_AUTO_PON_DRF_POF_DSLP		0xA7

/*
 * R83h (PTL) - partial window mode
 */
#define JD79661_PTL_PMODE_EN			BIT(0)

/* Time constants in ms, see section 9.1 "Power On/Off and DSLP Sequence" */
#define JD79661_RESET_DELAY			200U
#define JD79661_RESET_PULSE			2U
#define JD79661_PON_DELAY			100U
#define JD79661_BUSY_DELAY			5U

/*
 * R61h (TRES): Resolution setting.
 *
 * HRES and VRES are conceptually 10-bit values, but the driver only
 * ever programs the low bits: horizontal resolution must be a
 * multiple of 4 (the bottom two bits of HRES are hardwired to 0) and
 * the panels supported by this driver never exceed 511 lines, so the
 * top two bits of VRES are always 0 as well.
 */
struct jd79661_tres {
	uint8_t hres_hi;	/* HRES[9:8] */
	uint8_t hres_lo;	/* HRES[7:2] in bits 7:2, 0 in bits 1:0 */
	uint8_t vres_hi;	/* VRES[9:8] */
	uint8_t vres_lo;	/* VRES[7:0] */
} __packed;

BUILD_ASSERT(sizeof(struct jd79661_tres) == 4);

/*
 * R83h (PTL): Partial window.
 *
 * Same 10-bit convention as jd79661_tres for the horizontal
 * boundaries (bottom two bits hardwired to 0).
 */
struct jd79661_ptl {
	uint8_t hrst_hi;	/* HRST[9:8] */
	uint8_t hrst_lo;	/* HRST[7:2] in bits 7:2, 0 in bits 1:0 */
	uint8_t hred_hi;	/* HRED[9:8] */
	uint8_t hred_lo;	/* HRED[7:2] in bits 7:2, 0 in bits 1:0 */
	uint8_t vrst_hi;	/* VRST[9:8] */
	uint8_t vrst_lo;	/* VRST[7:0] */
	uint8_t vred_hi;	/* VRED[9:8] */
	uint8_t vred_lo;	/* VRED[7:0] */
	uint8_t pmode;		/* JD79661_PTL_PMODE_EN */
} __packed;

BUILD_ASSERT(sizeof(struct jd79661_ptl) == 9);

/*
 * R03h (PFS): Power-Off Sequence setting.
 *
 * Not documented in this datasheet excerpt, but the sibling UC81xx
 * family (see UC81XX_CMD_PFS in uc81xx_regs.h) uses the same address
 * for "PFS", a power-off-sequence setting; the 3-byte value below is
 * plausibly a wider variant of it, reproduced verbatim from the
 * vendor reference driver (Waveshare's EPD_2in15g.c).
 */
#define JD79661_CMD_PFS				0x03

#define JD79661_PFS_VAL0			0x10
#define JD79661_PFS_VAL1			0x54
#define JD79661_PFS_VAL2			0x44

/*
 * Undocumented commands issued by the vendor reference driver
 * (Waveshare's EPD_2in15g.c) during initialization. Their function is
 * not covered by the JD79661AA datasheet: 0x4Dh appears elsewhere in
 * the datasheet only as part of the one-time MTP-programming
 * flowchart (section 8.2.29, "R4Dh=78"), and 0xB4h/0xB5h/0xE0h/0xE7h/
 * 0xE9h do not appear in the datasheet at all. They are reproduced
 * here, with the exact values used by the reference driver, because
 * real panels have been observed to require them despite not being
 * documented. (0xE0h is the same address as UC81XX_CMD_CCSET in the
 * sibling UC81xx family, in uc81xx_regs.h - plausibly related, but
 * unconfirmed here.)
 */
#define JD79661_CMD_UNDOC_4D			0x4D
#define JD79661_CMD_UNDOC_B4			0xB4
#define JD79661_CMD_UNDOC_B5			0xB5
#define JD79661_CMD_UNDOC_E0			0xE0
#define JD79661_CMD_UNDOC_E7			0xE7
#define JD79661_CMD_UNDOC_E9			0xE9

#define JD79661_UNDOC_4D_VAL			0x78
#define JD79661_UNDOC_B4_VAL			0xD0
#define JD79661_UNDOC_B5_VAL			0x03
#define JD79661_UNDOC_E0_VAL			0x00
#define JD79661_UNDOC_E7_VAL			0x1C
#define JD79661_UNDOC_E9_VAL			0x01

/*
 * R41h (TSE) and R65h (GSST) are documented commands (see the
 * register table above) that the vendor reference driver
 * (Waveshare's EPD_2in15g.c) always writes with fixed values during
 * initialization, with no per-board override. Reproduced here
 * verbatim, same rationale as the JD79661_CMD_UNDOC_* values above.
 */
#define JD79661_TSE_VAL				0x00

/* First gate/source position: both S and G start at 0. */
#define JD79661_GSST_VAL0			0x00
#define JD79661_GSST_VAL1			0x00
#define JD79661_GSST_VAL2			0x00
#define JD79661_GSST_VAL3			0x00

/*
 * Native 2-bit color codes (R10h/DTM pixel values), matching the
 * vendor reference driver's EPD_2IN15G_BLACK/WHITE/YELLOW/RED
 * (Waveshare's EPD_2in15g.h) rather than a plain grayscale ramp:
 * this is a quad-tone (black/white/yellow/red) panel, not 4 shades
 * of gray. The actual color each code produces is whatever waveform
 * the on-chip MTP LUT has programmed for that gray level (R10h/DTM
 * in the datasheet calls the codes "Gray0".."Gray3"), not a fixed
 * voltage - these values just have to match what the vendor driver
 * (and therefore the MTP LUT on panels built for it) expects.
 */
#define JD79661_COLOR_BLACK			0x0U
#define JD79661_COLOR_WHITE			0x1U
#define JD79661_COLOR_YELLOW			0x2U
#define JD79661_COLOR_RED			0x3U

/* Replicate a 2-bit color code across all 4 pixels of a native byte. */
#define JD79661_COLOR_BYTE(c) (((c) << 6) | ((c) << 4) | ((c) << 2) | (c))

/*
 * R06h (BTST) and R01h (PWR) are board-specific tuning values with no
 * single "correct" layout worth hardcoding a struct for (they are
 * only ever needed to override the panel's MTP defaults). The driver
 * passes them through verbatim as raw byte arrays sourced from
 * devicetree ("softstart" and "pwr" properties respectively), same as
 * the UC81xx driver this one was derived from.
 */

#endif /* ZEPHYR_DRIVERS_DISPLAY_JD79661_REGS_H_ */
