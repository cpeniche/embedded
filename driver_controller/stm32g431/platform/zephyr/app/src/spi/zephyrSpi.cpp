/*
 * Spi.cpp
 *
 *  Created on: May 8, 2024
 *      Author: carlo
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>

#include "zephyrSpi.h"

template <class datatype, class errortype>
void zephyrSpi<datatype, errortype>::Init()
{
}

template <class datatype, class errortype>
void zephyrSpi<datatype, errortype>::Read(datatype *buffer, size_t size)
{
}

template <class datatype, class errortype>
void zephyrSpi<datatype, errortype>::Write(datatype *buffer, size_t size)
{
  setTxBuffer(buffer);
  setDataLength(size);
  this->error = spi_write(this->spiDevice, &spi_cfg, &txBuffer);
}

template <class datatype, class errortype>
errortype
zephyrSpi<datatype, errortype>::GetError()
{
  return 0;
}

template class zephyrSpi<uint8_t, int16_t>;