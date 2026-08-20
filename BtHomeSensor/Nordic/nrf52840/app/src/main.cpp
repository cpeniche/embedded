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

#define ENABLE_BLE

bmp280_sensors_data_t sensorsData;
uint32_t bmp280ErrorCounter = 0;
bmp280_handle_t handle;
bool bmp280_initialized = false;
static bool forceUpdate = false;

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
	bmp280_error_code_t InitializeAndConfigureBMP280(void);

	aht20 *aht20Sensor = aht20::getInstance();
#ifdef ENABLE_BLE			
	if ((err = initBluetooth()) < 0)
		LOG_ERR("Bluetooth Init Error = %" PRIo8, err);
#endif		

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
			if ((error = InitializeAndConfigureBMP280()) != BMP280_ERROR_OK)
			{
				bmp280_initialized = false;				
				LOG_ERR("BMP280 Initialization Error = %" PRIu32, error);
				display_bmp280_dashes();
			}
			else
			{
				bmp280_initialized = true;	
				bmp280ErrorCounter = 0;	
				forceUpdate=true;			
				LOG_DBG("BMP280 Initialized");
			}
		}
		
		if(bmp280_initialized)
		{						
			if (bmp280_get_all(&handle, &sensorsData) != BMP280_ERROR_OK)
			{	
				
				bmp280ErrorCounter++;
				if (bmp280ErrorCounter > 10)
				{
					bmp280_initialized = false;	
					bmp280ErrorCounter = 10;	
					forceUpdate=true;				
					display_bmp280_dashes();
				}				
				LOG_ERR("bmp280 Reading error");				
			}
			else
			{
				bmp280_calculate_altitude_hypsometric(&handle, &altitudeHypsometric, sensorsData.pressure, sensorsData.temperature);
				bmp280ErrorCounter=0;
				bTemp = (uint16_t)(sensorsData.temperature * 100);
				bPress = (uint32_t)(sensorsData.pressure);
				bHum = (uint32_t)(aht20Sensor->ReadHumidity() * 100);
#ifdef ENABLE_BLE				
				updateBluetoothData((uint8_t *)&bTemp, (uint8_t *)&bPress, (uint8_t *)&bHum);
#endif			
				display_update_readings(sensorsData.temperature,
																sensorsData.pressure,
																aht20Sensor->ReadHumidity(),
															  forceUpdate);

			  forceUpdate = false;
				LOG_DBG("[bmp20]Temperature = %f °C", (double)sensorsData.temperature);
				LOG_DBG("[bmp20]Pressure = %" PRIu32 "Pa", sensorsData.pressure);
				LOG_DBG("[bmp20]Altitude (quick) = %f m", (double)sensorsData.altitude);
				LOG_DBG("[bmp20]Altitude (hypsometric) = %f m", (double)altitudeHypsometric);
			}		
		}

		/*******************************************************/
		if(aht20Sensor->getStatus() == aht20::INITIALIZED)
		{
			if(aht20Sensor->TriggerMeasurement(DelayFunc, 100) == 0)
			{

				LOG_DBG("*************************************");
				LOG_DBG("[aht20] Humidity = %f %%", (double)aht20Sensor->ReadHumidity());
				LOG_DBG("[aht20] Temperature = %f °C", (double)aht20Sensor->ReadTemperature());
			}
			else
			{
				uint8_t count = aht20Sensor->getErrorCount();
				aht20Sensor->setErrorCount(++count);
				if(aht20Sensor->getErrorCount() > aht20::MAXERROR)
				{
					aht20Sensor->setStatus(aht20::UNINITIALIZED);
					display_aht20_dashes();
				}
			}
		}
		else
		{
			if(aht20Sensor->Init()==0)
			{
				aht20Sensor->setStatus(aht20::INITIALIZED);
				forceUpdate=true;
			}
			else
			{
				display_aht20_dashes();	
		 		LOG_ERR("Error Initializing aht20 Sensor");
			}
		}
		k_sleep(K_SECONDS(2));
	}
	return 0;
}

void DelayFunc(uint32_t ms)
{
	k_msleep((int32_t)ms);
}

bmp280_error_code_t InitializeAndConfigureBMP280()
{
	bmp280_error_code_t error = BMP280_ERROR_OK;
	LOG_DBG("Initializing BMP280");
		
	if( (error = bmp280_init(&handle, BMP280_I2C, BMP280_I2C_ADDRESS_2) )!= BMP280_ERROR_OK)
		goto exit;
	if((error = bmp280_set_mode(&handle, BMP280_MODE_NORMAL)) != BMP280_ERROR_OK)
		goto exit;
	if((error = bmp280_set_temperature_oversampling(&handle, BMP280_OVERSAMPLING_4X)) != BMP280_ERROR_OK)
		goto exit;
	if((error = bmp280_set_pressure_oversampling(&handle, BMP280_OVERSAMPLING_16X)) != BMP280_ERROR_OK)
		goto exit;
	if((error = bmp280_set_standby_time(&handle, BMP280_T_STANDBY_250MS)) != BMP280_ERROR_OK)
		goto exit;
	if((error = bmp280_set_filter_coefficient(&handle, BMP280_FILTER_16X)) != BMP280_ERROR_OK)
		goto exit;

exit:
	return error;

}
