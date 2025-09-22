#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include "spiConfig.h"

#define configBUFFERSIZE 16
const struct device *spi1_dev = DEVICE_DT_GET(spi1);

struct spi_config spi_cfg{
    .frequency = 1000000U,
    .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8),
};

/* array of buffers*/
struct spi_buf bufs[] = {
    {.buf = txMemoryBuffer,
     .len = configBUFFERSIZER},
}

/* transmit buffers*/
struct spi_buf_set txBuffer = {
    .buffers = bufs, /* buffer pointer */
    .count = 1       /* one buffer */
};

int sendDrvMsg(void)
{
  memset(txMemoryBuffer, 0xA5, sizeof(txMemoryBuffer));
  return spi_write(spi1_dev, spi_cfg, txBuffer);
}