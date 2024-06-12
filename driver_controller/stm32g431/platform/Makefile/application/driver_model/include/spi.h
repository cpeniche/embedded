/*
 * Spi.h
 *
 *  Created on: May 8, 2024
 *      Author: carlo
 */

#ifndef APPLICATION_DRIVER_MODEL_SRC_SPI_H_
#define APPLICATION_DRIVER_MODEL_SRC_SPI_H_
#include "driver.h"
#include "halIncludes.h"

template<class datatype,class errortype>
class Spi: public Driver<datatype,errortype>
{
public:

	void Init() override;
	void Read(datatype *) override;
	void Write(datatype *) override;
	errortype GetError() override;
	SPI_HandleTypeDef *get_handle();
	void set_data_lenght(uint32_t size){this->size = size;};
	void set_timeout(uint32_t timeout){this->timeout=timeout;};

private:
	SPI_HandleTypeDef drv_handle;
	datatype *rx_buffer;
	datatype *tx_buffer;
	uint16_t size;
	uint32_t timeout;
	errortype error;

	};

//#include "../src/spi.cpp"
#endif /* APPLICATION_DRIVER_MODEL_SRC_SPI_H_ */
