
#include <string.h>
#include <inttypes.h>
#include "spidrivermodel.h"
#include "main.h"
#include "drivers/include/espspi.h"


/// @brief /////////////////////
void *EspSpiBuilder::xBuild()
{

  EspSpiBusConfiguratorBuilder xprvEspSpiBusConfiguration;
  EspSpiDeviceBuilder xprvEspSpiDevice;
  EspSpiTransactionBuilder xprEspvSpiTransaction(xTxBuffer, xRxBuffer);
  xprvEspBusConfig = xprvEspSpiBusConfiguration.xBuild();

  spi_bus_initialize(xprvHost, &xprvEspBusConfig, xprvDmaChannel);
  spi_bus_add_device(xprvHost, &xprvEspDevice, &xprvSpiHandle);
  return new EspSpiDriver(xprvHost, xprvSpiHandle, xprvEspTransaction);
}

void EspSpiDriver::Init()
{

  spi_bus_config_t buscfg =
  {
#ifdef configREMOTE
  .mosi_io_num = -1,
#else
  .mosi_io_num = PIN_NUM_MOSI,
#endif
  .miso_io_num = PIN_NUM_MISO,
  .sclk_io_num = PIN_NUM_CLK,
  .quadwp_io_num = -1,
  .quadhd_io_num = -1,
  .max_transfer_sz = NUM_BITS
  };

/* define spi device configuration */
  spi_device_interface_config_t devcfg =
  {
#ifdef configREMOTE
    .mode = 2,
#else
    .mode = 1,    
#endif                     // SPI mode 0
    .clock_speed_hz = 1 * 1000 * 1000, // Clock out at 1 MHz
#ifdef configREMOTE
    .spics_io_num = -1, // CS pin
#else
    .spics_io_num = TLEMOTORCHIPSELECT, // CS pin
    .flags = SPI_DEVICE_TXBIT_LSBFIRST | SPI_DEVICE_RXBIT_LSBFIRST,
#endif
    .queue_size = 1, // We want to be able to queue 7 transactions at a time
    .pre_cb = NULL   // Specify pre-transfer callback to handle D/C line
  };
}

void EspSpiDriver::Transmit()
{
  error = spi_device_polling_transmit(xHandle, &xTransaction);
}

void *EspSpiDriver::GetReceiveData()
{
  return xTransaction.rx_buffer;
}

void *EspSpiDriver::GetError()
{
  return (void *)error;
}
