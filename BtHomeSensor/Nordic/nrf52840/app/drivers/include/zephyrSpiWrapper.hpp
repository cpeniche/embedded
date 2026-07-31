#pragma once
#include <zephyr/drivers/spi.h>
#include "busInterface.hpp"

class zephyrSpiWrapper : public busInterface
{
public:
  zephyrSpiWrapper(void){};
  int8_t Write(uint8_t *buffer, size_t len) override;
  int8_t Read(uint8_t *buffer, size_t len) override;
  int8_t WriteRead(uint8_t *txbuffer, size_t txlen, uint8_t *rxbuffer, size_t rxlen) override;  
  void SetDevice(void *);
  bool deviceReady() { return spi_is_ready_dt(_device); }

private:
  struct spi_dt_spec *_device;
  struct spi_buf_set _txBufs;
  struct spi_buf_set _rxBufs;
};
