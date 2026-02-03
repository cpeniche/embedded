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

protected:
	zephyrSpi(){};
	/*Static member declaration */	
	static zephyrSpi *instance_;

public:
	
	zephyrSpi(zephyrSpi &other) = delete;
	void operator=(const zephyrSpi &) = delete;
	static zephyrSpi *getInstance();	
	void Init() override{};
	void Read(datatype *, size_t) override;
	void Write(datatype *, size_t) override;
	void Configure(void *config) override;
	errortype GetError() override;
	void setDataLength(size_t size) { this->bufs.len = size; };
	void setBuffer(datatype *buffer) { this->bufs.buf = buffer; };

private:
	
	errortype error;
	struct spi_config *spi_cfg;

/* array of buffers*/
struct spi_buf bufs;

/* transmit buffers*/
struct spi_buf_set Buffer = {
		.buffers = &bufs, /* buffer pointer */
		.count = 1				/* one buffer */
};

const struct device *spiDevice = DEVICE_DT_GET(DT_NODELABEL(spi1));
};