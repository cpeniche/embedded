
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

  /* Create Hardware interface SetUp*/
  EspSpiBusSetUp xprvEspSpiBusSetUp(xprvEspBusConfig);
  xprvEspSpiBusSetUp.vSetClocK(PIN_NUM_CLK);
  xprvEspSpiBusSetUp.vSetMiso(PIN_NUM_MISO);
  xprvEspSpiBusSetUp.vSetMaxTransfer(NUM_BITS);
#ifdef configREMOTE
  xprvEspSpiBusSetUp.vSetMosi(GPIO_NUM_NC);
#else
  xprvEspSpiBusSetUp.vSetMosi(PIN_NUM_MOSI);
#endif

  /* Create Device SetUP*/
  EspSpiDeviceSetUp xprvEspSpiDevice(xprvEspDevice);
 
  xprvEspSpiDevice.vSetClockSpeed(1 * 1000 * 1000);
#ifdef configREMOTE
  xprvEspSpiDevice.vSetChipSelect(GPIO_NUM_NC);
  xprvEspSpiDevice.vSetMode(2);
#else
  xprvEspSpiDevice.vSetChipSelect(GPIO_NUM_13);
  xprvEspSpiDevice.vSetMode(1);
  xprvEspSpiDevice.vSetFlags(SPI_DEVICE_TXBIT_LSBFIRST | SPI_DEVICE_RXBIT_LSBFIRST);
#endif
  xprvEspSpiDevice.vSetQueueSize(1);
  xprvEspSpiDevice.vSetCallBackFunction(nullptr);

  /* Create Transaction SetUp*/
  EspSpiTransactionSetUp xprEspvSpiTransaction(xTxBuffer, xRxBuffer, xprvEspTransaction);
  xprEspvSpiTransaction.vsetLength(16);
  xprEspvSpiTransaction.vsetUser(nullptr);
  xprEspvSpiTransaction.vsetReceiveLength(16);
  xprEspvSpiTransaction.vsetReceivedBuffer(xRxBuffer);
  xprEspvSpiTransaction.vsetTransmitBuffer(xTxBuffer);
}

Spi *EspSpiBuilder::xBuild()
{

  return dynamic_cast<Spi *>(new EspSpi(xprvEspTransaction, xprvEspBusConfig, xprvEspDevice));
}

void EspSpi::Init()
{

  error = spi_bus_initialize(SPI2_HOST, &xprvBusConfig, SPI_DMA_CH_AUTO);
  if (error == ESP_OK)
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
  return static_cast<void *>(&error);
}
