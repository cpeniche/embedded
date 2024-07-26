
#include <string.h>
#include <inttypes.h>
#include "spidrivermodel.h"
#include "main.h"
#include "driver/gpio.h"
#include "drivers/include/espspi.h"

/// @brief /////////////////////

EspSpiBuilder::EspSpiBuilder(uint8_t *xTxBuffer,
                             uint8_t *xRxBuffer)
{
  EspSpiBusSetUp xprvEspSpiBusSetUp(xprvEspBusConfig);
  xprvEspSpiBusSetUp.vSetClocK(PIN_NUM_CLK);
  xprvEspSpiBusSetUp.vSetMiso(PIN_NUM_MISO);
  xprvEspSpiBusSetUp.vSetMosi(PIN_NUM_MOSI);
  xprvEspSpiBusSetUp.vSetMaxTransfer(NUM_BITS);
#ifdef configREMOTE
  xprvEspSpiBusSetUp.vSetMosi(-1);
#endif

  EspSpiDeviceSetUp xprvEspSpiDevice(xprvEspDevice);

  xprvEspSpiDevice.vSetChipSelect(GPIO_NUM_1);
  xprvEspSpiDevice.vSetMode(1);
  xprvEspSpiDevice.vSetFlags(SPI_DEVICE_TXBIT_LSBFIRST | SPI_DEVICE_RXBIT_LSBFIRST);
#ifdef configREMOTE
  xprvEspSpiDevice.vSetChipSelect(-1);
  xprvEspSpiDevice.vSetMode(2);
#endif
  xprvEspSpiDevice.vSetQueueSize(1);
  xprvEspSpiDevice.vSetCallBackFunction(nullptr);

  EspSpiTransactionSetUp xprEspvSpiTransaction(xTxBuffer, xRxBuffer, xprvEspTransaction);

  xprEspvSpiTransaction.vsetLength(16);
  xprEspvSpiTransaction.vsetUser(nullptr);
  xprEspvSpiTransaction.vsetReceiveLength(16);
  xprEspvSpiTransaction.vsetReceivedBuffer(xRxBuffer);
  xprEspvSpiTransaction.vsetTransmitBuffer(xTxBuffer);
}

Spi *EspSpiBuilder::xBuild()
{

  // EspSpi xprvEspSpi(xprvEspTransaction, xprvEspBusConfig, xprvEspDevice);
  // Spi *xprReturnSpi = dynamic_cast<Spi *>(new EspSpi(xprvEspTransaction, xprvEspBusConfig, xprvEspDevice));
  // return xprReturnSpi;
  return dynamic_cast<Spi *>(new EspSpi(xprvEspTransaction, xprvEspBusConfig, xprvEspDevice));
}

void EspSpi::Init()
{

  error = spi_bus_initialize(SPI2_HOST, &xprvBusConfig, SPI_DMA_CH_AUTO);
  error = spi_bus_add_device(SPI2_HOST, &xDeviceSetUp, &xHandle);
 }

void EspSpi::Transmit()
{
  error = spi_device_polling_transmit(xHandle, &xTransaction);
}

void *EspSpi::GetReceiveData()
{
  return xTransaction.rx_buffer;
}

void *EspSpi::GetError()
{
  return (void *)error;
}
