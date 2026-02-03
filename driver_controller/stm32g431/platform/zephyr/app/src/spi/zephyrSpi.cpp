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

/* Static member class memory allocation */
template <class datatype, class errortype>
zephyrSpi<datatype, errortype> *zephyrSpi<datatype, errortype>::instance_ = nullptr;

template <class datatype, class errortype>
zephyrSpi<datatype, errortype> *zephyrSpi<datatype, errortype>::getInstance()
{
  if(instance_==nullptr)
  {
    instance_=new zephyrSpi<datatype, errortype>();
  }
  return instance_;
}

template <class datatype, class errortype>
void zephyrSpi<datatype, errortype>::Read(datatype *buffer, size_t size)
{
  setBuffer(buffer);
  setDataLength(size);
  this->error = spi_read(this->spiDevice, spi_cfg, &Buffer);
}

template <class datatype, class errortype>
void zephyrSpi<datatype, errortype>::Write(datatype *buffer, size_t size)
{

  setBuffer(buffer);
  setDataLength(size);
  this->error = spi_write(this->spiDevice, spi_cfg, &Buffer);
}

template <class datatype, class errortype>
void zephyrSpi<datatype, errortype>::Configure(void *config)
{
  this->spi_cfg = static_cast<struct spi_config *>(config);
}

template <class datatype, class errortype>
errortype
zephyrSpi<datatype, errortype>::GetError()
{
  return 0;
}

template class zephyrSpi<uint8_t, int16_t>;