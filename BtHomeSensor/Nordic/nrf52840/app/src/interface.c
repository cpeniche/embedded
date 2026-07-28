/*BMP280 barometric pressure and temperature sensor C Driver*/
/*Reza Ebrahimi - https://github.com/ebrezadev */
/*Version 5.0*/
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>
#include "bmp280.h"
#include "interface.h"
#include <math.h>

#define I2C_SPEED 400000
struct i2c_dt_spec device = I2C_DT_SPEC_GET(DT_NODELABEL(bmp280));

/*writes an array (data[]) of arbitrary size (dataLength) to I2C address (deviceAddress), starting from an internal register address (startRegisterAddress)*/
int bmp280_write_array(uint8_t deviceAddress, uint8_t startRegisterAddress, uint8_t *data, uint8_t dataLength)
{
	uint8_t buffer[20] = {0};
	buffer[0] = startRegisterAddress;
	memcpy(&buffer[1], data, dataLength);
	return i2c_write_dt(&device, buffer, dataLength + 1);
}

/*reads an array (data[]) of arbitrary size (dataLength) from I2C address (deviceAddress), starting from an internal register address (startRegisterAddress)*/
int bmp280_read_array(uint8_t deviceAddress, uint8_t startRegisterAddress, uint8_t *data, uint8_t dataLength)
{
	int8_t err = 0;

	if ((err = i2c_write_dt(&device, &startRegisterAddress, 1)) < 0)
	{
		printk("I2C Write error code %d", err);
		return err;
	}

	if ((err = i2c_read_dt(&device, data, dataLength)) < 0)
	{
		printk("I2C Read error code %d", err);
		return err;
	}

	return err;
}

/*initiates the I2C peripheral and sets its speed*/
int bmp280_i2c_init(uint8_t deviceAddress)
{
	if (!i2c_is_ready_dt(&device))
	{
		printk("I2C device not ready\n");
		return -1;
	}
	return 0;
}

/*initiates the I2C peripheral and sets its speed*/
int bmp280_i2c_deinit(uint8_t deviceAddress)
{
	return 0;
}

/*a delay function for milliseconds delay*/
int delay_function(uint32_t delayMS)
{
	k_msleep(delayMS);
	return 0;
}

/*implements a power function (used in altitude calculation)*/
int power_function(float x, float y, float *result)
{
	*result = pow(x, y);

	return 0;
}

int getMode()
{
	return DT_PROP(DT_NODELABEL(bmp280), mode);
}
