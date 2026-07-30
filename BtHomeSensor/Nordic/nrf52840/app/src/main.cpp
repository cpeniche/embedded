/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main_app, LOG_LEVEL_DBG);
#include "bmp280.h"
#include "aht20.hpp"
#include "interface.h"
#include "bluetooth.h"

bmp280_sensors_data_t sensorsData;
bmp280_handle_t handle;

void DelayFunc(uint32_t ms);

int main(int argc, char *argv[])
{

	uint16_t bTemp = 0;
	uint32_t bPress = 0;
	int8_t err = 0;
	aht20 *aht20Sensor = aht20::getInstance();

	if ((err = initBluetooth()) < 0)
		LOG_ERR("Bluetooth Init Error = %" PRIo8, err);

	handle.dependency_interface.bmp280_interface_init = bmp280_i2c_init;
	handle.dependency_interface.bmp280_interface_deinit = bmp280_i2c_deinit;
	handle.dependency_interface.bmp280_write_array = bmp280_write_array;
	handle.dependency_interface.bmp280_read_array = bmp280_read_array;
	handle.dependency_interface.bmp280_delay_function = delay_function;
	handle.dependency_interface.bmp280_power_function = power_function;

	bmp280_error_code_t error = bmp280_init(&handle, BMP280_I2C, BMP280_I2C_ADDRESS_2);

	aht20Sensor->TriggerMeasurement(DelayFunc, 100);

	if (error != BMP280_ERROR_OK)
	{
		LOG_ERR("FAIL");

		for (;;)
			;
	}

	LOG_DBG("SUCCESS");

	error = bmp280_set_mode(&handle, BMP280_MODE_NORMAL);
	error = bmp280_set_temperature_oversampling(&handle, BMP280_OVERSAMPLING_4X);
	error = bmp280_set_pressure_oversampling(&handle, BMP280_OVERSAMPLING_16X);
	error = bmp280_set_standby_time(&handle, BMP280_T_STANDBY_250MS);
	error = bmp280_set_filter_coefficient(&handle, BMP280_FILTER_16X);

	while (1)
	{

		bmp280_error_code_t error = bmp280_get_all(&handle, &sensorsData);

		if (error != BMP280_ERROR_OK)
		{
			LOG_ERR("FAIL");

			for (;;)
				;
		}

		float altitudeHypsometric;

		bmp280_calculate_altitude_hypsometric(&handle, &altitudeHypsometric, sensorsData.pressure, sensorsData.temperature);
		bTemp = (uint16_t)(sensorsData.temperature * 100);
		bPress = (uint32_t)(sensorsData.pressure);
		updateBluetoothData((uint8_t *)&bTemp, (uint8_t *)&bPress);
		LOG_DBG("Temperature = %f °C", (double)sensorsData.temperature);
		LOG_DBG("Pressure = %" PRIu32 "Pa", sensorsData.pressure);
		LOG_DBG("Altitude (quick) = %f m", (double)sensorsData.altitude);
		LOG_DBG("Altitude (hypsometric) = %f m", (double)altitudeHypsometric);

		k_sleep(K_SECONDS(2));
	}
	return 0;
}

void DelayFunc(uint32_t ms)
{
	k_msleep((int32_t)ms);
}