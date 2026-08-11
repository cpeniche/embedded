#pragma once

#include <stdint.h>

/*
 * Publishes the latest sensor readings to the display thread. Non-blocking -
 * safe to call from any thread. Only the most recent reading is kept; the
 * display thread redraws a label only when its rounded integer value has
 * changed.
 */
void display_update_readings(float temperature_c, uint32_t pressure_pa, float humidity_pct);
