#include "zephyrSpiWrapper.hpp"

int8_t zephyrSpiWrapper::Write(uint8_t *buffer, size_t len)
{
 
  int8_t err=0;
  struct spi_buf buffers={.buf=buffer,.len=len};

  _txBufs.buffers=&buffers;
  _txBufs.count=1;


  if(!(err=spi_is_ready_dt(_device)))
    return err;
  return spi_write_dt(_device,&_txBufs);
}

int8_t zephyrSpiWrapper::Read(uint8_t *buffer, size_t len)
{
  
  int8_t err=0;

  struct spi_buf buffers={.buf=buffer,.len=len};

  _rxBufs.buffers=&buffers;
  _rxBufs.count=1;

  if(!(err=spi_is_ready_dt(_device)))
    return err;
  return spi_read_dt(_device,&_rxBufs);
}

int8_t zephyrSpiWrapper::WriteRead(uint8_t *txbuffer, size_t txlen, uint8_t *rxbuffer, size_t rxlen)
{

  int8_t err=0;

  struct spi_buf bufferstx={.buf=txbuffer,.len=txlen};
  struct spi_buf buffersrx={.buf=rxbuffer,.len=rxlen};

  _txBufs.buffers=&bufferstx;
  _txBufs.count=1;

  _rxBufs.buffers=&buffersrx;
  _rxBufs.count=1;


  if(!(err=spi_is_ready_dt(_device)))
    return err;
  return spi_transceive_dt(_device,&_txBufs,&_rxBufs);
}

void zephyrSpiWrapper::SetDevice(void *device)
{
  _device = (struct spi_dt_spec*)device;
}
