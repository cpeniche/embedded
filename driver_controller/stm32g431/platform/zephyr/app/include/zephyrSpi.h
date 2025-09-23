/*
 * zephyrSpi.h
 *
 *  Created on: May 8, 2024
 *      Author: carlo
 */
#pragma once
#include "spiInterface.h"

template <class datatype, class errortype>
class zephyrSpi : public spiInterface<datatype, errortype>
{
public:
	void Init() override;
	void Read(datatype *) override;
	void Write(datatype *) override;
	errortype GetError() override;
	void setDataLength(size_t size) { this->bufs.len = size; };
	void setTxBuffer(datatype *buffer) { this->bufs.buf = buffer; };
	// void setRxBuffer(datatype *buffer) { this->rx_buffer = buffer; };

private:
	// datatype *rx_buffer;
	// datatype *tx_buffer;
	errortype error;
	struct spi_config spi_cfg = {
			.frequency = 1000000U,
			.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8),
	};

	/* array of buffers*/
	struct spi_buf bufs;

	/* transmit buffers*/
	struct spi_buf_set txBuffer = {
			.buffers = &bufs, /* buffer pointer */
			.count = 1			 /* one buffer */
	};
};