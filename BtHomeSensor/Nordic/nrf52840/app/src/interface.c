/*BMP280 barometric pressure and temperature sensor C Driver*/
/*Reza Ebrahimi - https://github.com/ebrezadev */
/*Version 5.0*/
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "nrfI2c.h"
#include "interface.h"
#include <math.h>

#define I2C_SPEED 400000
// struct i2c_dt_spec device = I2C_DT_SPEC_GET(DT_NODELABEL(bmp280));

/*writes an array (data[]) of arbitrary size (dataLength) to I2C address (deviceAddress), starting from an internal register address (startRegisterAddress)*/
int bmp280_write_array(uint8_t deviceAddress, uint8_t startRegisterAddress, uint8_t *data, uint8_t dataLength)
{
	uint8_t buffer[20] = {0};
	buffer[0] = startRegisterAddress;
	memcpy(&buffer[1], data, dataLength);
	return cppI2cWrite(buffer, dataLength + 1);
}

/*reads an array (data[]) of arbitrary size (dataLength) from I2C address (deviceAddress), starting from an internal register address (startRegisterAddress)*/
int bmp280_read_array(uint8_t deviceAddress, uint8_t startRegisterAddress, uint8_t *data, uint8_t dataLength)
{
	int8_t err = 0;

	if ((err = cppI2cWrite(&startRegisterAddress, 1)) < 0)
		return err;
	return cppI2cRead(data, dataLength);
}

/*initiates the I2C peripheral and sets its speed*/
int bmp280_i2c_init(uint8_t deviceAddress)
{
	return cppIsI2cReady();
}

/*initiates the I2C peripheral and sets its speed*/
int bmp280_i2c_deinit(uint8_t deviceAddress)
{
	return 0;
}

/*a delay function for milliseconds delay*/
int delay_function(uint32_t delayMS)
{
	zephyrDelay(delayMS);
	return 0;
}

/*implements a power function (used in altitude calculation)*/
int power_function(float x, float y, float *result)
{
	*result = pow(x, y);

	return 0;
}
