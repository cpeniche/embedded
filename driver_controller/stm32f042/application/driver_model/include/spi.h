/*
 * Spi.h
 *
 *  Created on: May 8, 2024
 *      Author: carlo
 */

#ifndef APPLICATION_DRIVER_MODEL_SRC_SPI_H_
#define APPLICATION_DRIVER_MODEL_SRC_SPI_H_
#include "driver.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_spi.h"

class Spi: public Driver
{
public:
	void Init() override;
	void Read() override;
	void Write() override;
	SPI_HandleTypeDef *get_handle();

private:
	SPI_HandleTypeDef drv_handle;
};

#endif /* APPLICATION_DRIVER_MODEL_SRC_SPI_H_ */
