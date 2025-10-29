/*
 * zephyrSpi.h
 *
 *  Created on: May 8, 2024
 *      Author: carlo
 */
#pragma once
#include <zephyr/drivers/gpio.h>
#include "spiInterface.h"

template <class datatype, class errortype>
class zephyrSpi : public spiInterface<datatype, errortype>
{
public:
	void Init() override;
	void Read(datatype *, size_t) override;
	void Write(datatype *, size_t) override;
	errortype GetError() override;
	void setDataLength(size_t size) { this->bufs.len = size; };
	void setBuffer(datatype *buffer) { this->bufs.buf = buffer; };

private:
	errortype error;
	struct spi_config spi_cfg = {
			.frequency = 1000000U,
			.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8),
			.slave = 0x0
			/*	.cs ={nullptr,0}*/
	};

	/* array of buffers*/
	struct spi_buf bufs;

	/* transmit buffers*/
	struct spi_buf_set Buffer = {
			.buffers = &bufs, /* buffer pointer */
			.count = 1				/* one buffer */
	};

	const struct device *spiDevice = DEVICE_DT_GET(DT_NODELABEL(spi1));
};