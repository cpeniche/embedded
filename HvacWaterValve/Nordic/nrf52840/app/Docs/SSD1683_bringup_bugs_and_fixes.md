# SSD1683 E-Paper Display Bring-up: Bugs and Fixes

Summary of the issues found and fixed while bringing up the GoodDisplay/Waveshare
4.2" SSD1683-based e-paper panel (300x400 native, 4-grayscale-capable) on an
nRF52840 dongle running Zephyr + LVGL.

## 1. Missing SSD1683 devicetree binding *(historical - resolved upstream, see the 2026-08-11 update at the bottom)*

The pinned Zephyr revision didn't include `solomon,ssd1683.yaml`. Backported
verbatim from upstream Zephyr `main`.

- `zephyr/dts/bindings/display/solomon,ssd1683.yaml` (new)
- `zephyr/drivers/display/Kconfig.ssd16xx` (added `DT_HAS_SOLOMON_SSD1683_ENABLED`)

No longer needed: current upstream `main` ships the binding, the Kconfig
symbol, and the `quirks_solomon_ssd1683` driver entry natively, so these two
files are no longer part of the local patch.

## 2. Color inversion

The panel's black RAM plane is optically inverted relative to the
`PIXEL_FORMAT_MONO10` "1=black" contract the driver otherwise assumes.

**Fix:** added an `inverted` quirks flag; when set, the driver declares
`PIXEL_FORMAT_MONO01` instead, which flips how Zephyr's LVGL glue maps
foreground/background pixels to RAM bits.

## 3. "Buffer out of bounds (width)" for a 300px-wide panel

LVGL's internal `I1` (1bpp) color format unconditionally byte-rounds
invalidated X regions (e.g. 299 -> 303), which then tripped the driver's
strict bounds check against the true 300px panel width. Every previously
Zephyr-supported EPD panel happened to have a byte-aligned width, so this
class of bug had never been exercised before.

**Fix:** added a `full_width_only` quirks flag that sets
`SCREEN_INFO_X_ALIGNMENT_WIDTH`, forcing every flush to use the full logical
width instead of a partial, rounding-prone one.

## 4. "Buffer height not multiple of 8" regression

Caused by `SCREEN_INFO_X_ALIGNMENT_WIDTH` and `SCREEN_INFO_MONO_VTILED`
Y-rounding being mutually exclusive (`if`/`else`) in
`lvgl_rounder_cb_mono()` - enabling the first broke the second.

**Fix:** combined the two branches so VTILED Y-rounding still applies when
X_ALIGNMENT_WIDTH is also set.

- `zephyr/modules/lvgl/lvgl_display_mono.c`

## 5. First invalidation of the program's lifetime never reaches the rounder

Confirmed via direct instrumentation of `lv_inv_area()` and the rounder
callback: the very first full-screen invalidation of the whole program's
lifetime never reaches the rounder. It still gets permanently saved into
`disp->inv_areas[0]` with the wrong, byte-rounding-widened X bounds, and
`lv_inv_area()`'s own "skip if already covered" optimization then prevents
any later, correctly-rounded call from ever overwriting it.

**Fix:** burn a disposable invalidate+render cycle in `display.cpp` at boot,
before anything meaningful is on screen, so the *next* full-screen render is
the second invalidation of the program's lifetime (which does reach the
rounder correctly).

## 6. Diagonal shear on rendered content

`lvgl_transform_buffer()`'s stride computation didn't round the byte count
up to a whole byte before applying `LV_DRAW_BUF_STRIDE_ALIGN`, under-counting
the real per-row stride LVGL used for any width that isn't already a
multiple of 8 (e.g. 300px needs 38 bytes/row, not 300 bits). Reading with
the wrong stride drifted by a few bits more every row.

**Fix:** corrected the stride math to match LVGL's own `width_to_stride()`.

- `zephyr/modules/lvgl/lvgl_display_mono.c`

## 7. Blanking on/off no-op

`ssd16xx_init()` leaves `blanking_on = false`, so calling
`display_blanking_off()` alone was a no-op (it only refreshes if blanking
was already on).

**Fix:** explicit `display_blanking_on()` / `display_blanking_off()` pairing
around full-screen renders in `display.cpp`.

## 8. Stuck black background on full-screen refreshes with real content

The most involved issue. Two real, previously-unknown gaps in `ssd16xx.c`
(confirmed missing in upstream Zephyr too, not specific to this project):

**a. Missing `tssv` (temperature sensor selection).** Without it, the
`LOAD_TEMPERATURE` bit was never set in the "Display Update Control 2"
command, and the LUT was loaded via a hardcoded generic 25C fallback instead
of a real sensor-selected one. Found by computing the actual byte our driver
sends for that command (`0xD7`/`0xDF` for full/partial) and comparing
against Waveshare's official `EPD_4in2_V2.c` reference driver, which sends
`0xF7`/`0xFF` for the equivalent calls - the only difference being this bit.

**Fix:** added `tssv = <0x80>;` (internal sensor) to the devicetree node,
matching both Waveshare's reference and the real upstream `beaglebadge`
board (same chip family).

**b. Missing `SSD16XX_CMD_UPDATE_CTRL1` (`0x21`).** This register is defined
in `ssd16xx_regs.h` but was never written anywhere in `ssd16xx.c`, for any
panel, in this checkout or upstream. Waveshare's reference always sends it
right after soft reset. It configures how the second ("red") RAM plane is
used - normal vs. bypassed - and without it the controller was left at its
power-on default, which was factoring that second plane into every pixel's
drive computation.

**Fix:** added a new `bypass_red_ram` quirks flag. When set, the driver
sends `UPDATE_CTRL1` on every profile switch: `[0x40, 0x00]` (bypass red RAM)
for full refresh, `[0x00, 0x00]` (normal - red RAM holds the old-image
shadow needed for a proper differential drive) for partial refresh, matching
Waveshare's reference exactly for each mode.

Two dead-end hypotheses were explored and reverted along the way before
landing on the real cause above: an "OTP default is secretly a 4-grayscale
LUT" theory, and a `skip_red_ram_shadow` mechanism that appeared to fix the
uniform-content case only by coincidence and made real (non-uniform) content
worse.

- `zephyr/drivers/display/ssd16xx.c`
- `boards/lvgl-display-ssd1683.dtsi`

## 9. Grayish border after a full refresh

Missing `border-waveform` override, left at OTP/power-on default. Waveshare's
reference always explicitly sets it: `0x05` for full refresh, `0x80` for
partial refresh.

**Fix:** added `border-waveform` to both the `full {}` and `partial {}`
devicetree profiles.

## 10. Faded partial-refresh text

Partly explained by the missing `tssv` fix above (#8a), and partly a
transient regression introduced while chasing #8b: bypassing the red RAM
plane unconditionally for both full *and* partial refresh starved partial
refresh's differential waveform of the old-image data it needs.

**Fix:** made `bypass_red_ram` mode-aware (see #8b) so only full refresh
bypasses the red RAM plane.

## 11. Screen rotation showed content shifted, not rotated

`lv_display_set_rotation()` alone only swaps LVGL's *reported* resolution
for widget layout purposes - it doesn't rotate rendered pixels in this LVGL
version. Actual per-pixel rotation additionally requires matrix rotation
(`lv_display_set_matrix_rotation()`), which requires
`LV_DISPLAY_RENDER_MODE_FULL`. That render mode, however, makes
`lv_inv_area()` return before ever firing the `LV_EVENT_INVALIDATE_AREA`
event that our rounder callback (fixes #3/#4) depends on - silently
disabling those fixes.

**Fix:** reverted the LVGL-side rotation approach entirely. Instead fixed a
real gap in `ssd16xx_get_capabilities()`: it never swapped
`x_resolution`/`y_resolution` for `ROTATED_90`/`270` orientations, even
though the driver's own bounds-check in `ssd16xx_write()` already
transposes which axis gets validated against what for those orientations.
With that fixed, the devicetree's `rotation` property alone makes the driver
report the correct landscape resolution directly to LVGL, keeping the
existing (already debugged) `PARTIAL`-render-mode pipeline untouched.

- `zephyr/drivers/display/ssd16xx.c`
- `boards/lvgl-display-ssd1683.dtsi`
- `prj.conf`
- `src/display.cpp`

## 12. LVGL mono render-buffer under-allocation for non-byte-aligned widths

A second instance of the same byte-rounding root cause as #3/#6, in a
different function, previously missing from this changelog:
`lvgl_allocate_rendering_buffers()` in `modules/lvgl/lvgl.c` sized the MONO
render and conversion buffers as `buf_nbr_pixels / 8` - the flat total pixel
count rounded to a byte. LVGL's own per-row stride
(`width_to_stride()` in `lv_draw_buf.c`) rounds *each row's* byte count up
independently, which is strictly larger than that flat count whenever the
panel's width isn't itself a multiple of 8 (300px again), under-allocating
and overflowing both buffers once rendered into at more than one row's
height.

**Fix:** compute the buffer size as `rows * row_bytes + 8` (row count times
per-row byte stride) instead, matching LVGL's real per-row stride.

- `zephyr/modules/lvgl/lvgl.c`

## Reproducing on a fresh checkout

The changes above live in `/home/vscode/zephyr`, a separate git checkout not
tracked by this project. `patches/zephyr.patch` and `patches/apply_patches.sh`
(in this project's root) capture and reapply them - see
`patches/apply_patches.sh` for usage. The LVGL checkout
(`/home/vscode/modules/lib/gui/lvgl`) needs no patch: its only local change
was debugging instrumentation in `lv_refr.c`, since reverted.

**2026-08-11 update:** re-applied against a much newer upstream Zephyr
checkout (`main`, `v4.4.0-11134-gaeafe3af2a0`) that now natively includes
SSD1683 devicetree binding and quirks-table support
(`c8e3417804f "drivers: display: ssd16xx: add support for SSD1683"`) and has
been through several unrelated `ssd16xx.c` refactors since (MIPI DBI
conversion, `const` driver API structs, etc.), so the old patch's line
context no longer matched. `patches/zephyr.patch` was regenerated against
this checkout with the same logical changes as before - `inverted`,
`full_width_only`, and `bypass_red_ram` quirks flags on the native
`quirks_solomon_ssd1683` entry, the `ssd16xx_get_capabilities()` rotation/
pixel-format/screen-info changes, the `UPDATE_CTRL1` write in
`ssd16xx_set_profile()`, and the unchanged LVGL mono buffer-sizing/stride/
rounder fixes in `lvgl.c` and `lvgl_display_mono.c` (#3/#4/#6/#12) - minus
the devicetree binding and `Kconfig.ssd16xx` hunks (#1), which upstream now
provides natively and no longer need patching. This project's own files
(`boards/lvgl-display-ssd1683.dtsi`, `prj.conf`, `src/display.cpp`) needed
no changes - they already encoded `tssv`,
`border-waveform`, `rotation`, and the blanking/boot-invalidation
workarounds independently of the Zephyr-side driver.

## Done

- Diagnostic `printk`/`LOG_ERR` instrumentation added during debugging has
  been stripped from `ssd16xx.c` and `lvgl_display_mono.c`, and `lv_refr.c`
  reverted to pristine upstream (it had no other changes).
