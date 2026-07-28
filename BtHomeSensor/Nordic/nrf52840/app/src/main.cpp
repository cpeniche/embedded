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
#include "bmp280.h"
#include "interface.h"
#include "bluetooth.h"

bmp280_sensors_data_t sensorsData;
bmp280_handle_t handle;

int main(int argc, char *argv[])
{

	handle.dependency_interface.bmp280_interface_init = bmp280_i2c_init;
	handle.dependency_interface.bmp280_interface_deinit = bmp280_i2c_deinit;
	handle.dependency_interface.bmp280_write_array = bmp280_write_array;
	handle.dependency_interface.bmp280_read_array = bmp280_read_array;
	handle.dependency_interface.bmp280_delay_function = delay_function;
	handle.dependency_interface.bmp280_power_function = power_function;

	bmp280_error_code_t error = bmp280_init(&handle, BMP280_I2C, BMP280_I2C_ADDRESS_2);

	if (error != BMP280_ERROR_OK)
	{
		printk("FAIL");

		for (;;)
			;
	}

	printk("SUCCESS\n");

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
			printk("FAIL\n");

			for (;;)
				;
		}

		float altitudeHypsometric;

		bmp280_calculate_altitude_hypsometric(&handle, &altitudeHypsometric, sensorsData.pressure, sensorsData.temperature);

		printk("Temperature = %f °C\n", (double)sensorsData.temperature);
		printk("Pressure = %" PRIu32 "Pa\n", sensorsData.pressure);
		printk("Altitude (quick) = %f m\n", (double)sensorsData.altitude);
		printk("Altitude (hypsometric) = %f m\n", (double)altitudeHypsometric);

		k_sleep(K_SECONDS(2));
		// pressure->ReadDeviceCode(&code);
		// printk("BMP280 Device Code: %x\n", code);
	}
	return 0;
}