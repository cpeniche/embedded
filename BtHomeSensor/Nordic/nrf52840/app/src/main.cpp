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
LOG_MODULE_REGISTER(main_app, LOG_LEVEL_ERR);
#include "bmp280.h"
#include "aht20.hpp"
#include "interface.h"
#include "bluetooth.h"
#include "display.h"

bmp280_sensors_data_t sensorsData;
uint32_t bmp280ErrorCounter = 0;
bmp280_handle_t handle;
bool bmp280_initialized = false;

void DelayFunc(uint32_t ms);
void screenExample();

int main(int argc, char *argv[])
{

	uint16_t bTemp = 0;
	uint32_t bPress = 0;
	uint16_t bHum = 0;
	int err = 0;
	float altitudeHypsometric;
	bmp280_error_code_t error;

	aht20 *aht20Sensor = aht20::getInstance();

	/*if ((err = initBluetooth()) < 0)
		LOG_ERR("Bluetooth Init Error = %" PRIo8, err);*/

	handle.dependency_interface.bmp280_interface_init = bmp280_i2c_init;
	handle.dependency_interface.bmp280_interface_deinit = bmp280_i2c_deinit;
	handle.dependency_interface.bmp280_write_array = bmp280_write_array;
	handle.dependency_interface.bmp280_read_array = bmp280_read_array;
	handle.dependency_interface.bmp280_delay_function = delay_function;
	handle.dependency_interface.bmp280_power_function = power_function;

	while(1)
	{

		if (!bmp280_initialized)
		{
			LOG_DBG("Initializing BMP280");
		
			error = bmp280_init(&handle, BMP280_I2C, BMP280_I2C_ADDRESS_2);
			error = bmp280_set_mode(&handle, BMP280_MODE_NORMAL);
			error = bmp280_set_temperature_oversampling(&handle, BMP280_OVERSAMPLING_4X);
			error = bmp280_set_pressure_oversampling(&handle, BMP280_OVERSAMPLING_16X);
			error = bmp280_set_standby_time(&handle, BMP280_T_STANDBY_250MS);
			error = bmp280_set_filter_coefficient(&handle, BMP280_FILTER_16X);
			if (error != BMP280_ERROR_OK)
			{
				bmp280_initialized = false;				
				LOG_ERR("BMP280 Initialization Error = %" PRIu32, error);
				display_show_dashes();
			}
			else
			{
				bmp280_initialized = true;			
				LOG_DBG("BMP280 Initialized");
			}
		}
		
		if(bmp280_initialized)
		{
			
	  	error = bmp280_get_all(&handle, &sensorsData);
			error = bmp280_calculate_altitude_hypsometric(&handle, &altitudeHypsometric, sensorsData.pressure, sensorsData.temperature);
			
			if (error != BMP280_ERROR_OK)
			{	
				
				bmp280ErrorCounter++;
				if (bmp280ErrorCounter > 10)
				{
					bmp280_initialized = false;
					bmp280ErrorCounter = 0;
				}				
				LOG_ERR("bmp280 Reading error");
				display_show_dashes();
			}
			else{
				bTemp = (uint16_t)(sensorsData.temperature * 100);
				bPress = (uint32_t)(sensorsData.pressure);
				bHum = (uint32_t)(aht20Sensor->ReadHumidity() * 100);
				updateBluetoothData((uint8_t *)&bTemp, (uint8_t *)&bPress, (uint8_t *)&bHum);
				display_update_readings(sensorsData.temperature,
																sensorsData.pressure,
																aht20Sensor->ReadHumidity());
			
				LOG_DBG("[bmp20]Temperature = %f °C", (double)sensorsData.temperature);
				LOG_DBG("[bmp20]Pressure = %" PRIu32 "Pa", sensorsData.pressure);
				LOG_DBG("[bmp20]Altitude (quick) = %f m", (double)sensorsData.altitude);
				LOG_DBG("[bmp20]Altitude (hypsometric) = %f m", (double)altitudeHypsometric);
			}		
		}
		
		aht20Sensor->TriggerMeasurement(DelayFunc, 100);
		LOG_DBG("*************************************");
		LOG_DBG("[aht20] Humidity = %f %%", (double)aht20Sensor->ReadHumidity());
		LOG_DBG("[aht20] Temperature = %f °C", (double)aht20Sensor->ReadTemperature());
		
		k_sleep(K_SECONDS(2));
	}
	return 0;
}

void DelayFunc(uint32_t ms)
{
	k_msleep((int32_t)ms);
}
