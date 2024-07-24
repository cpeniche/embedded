
#include <string.h>
#include <inttypes.h>
#include "spidrivermodel.h"
#include "main.h"
#include "drivers/include/espspi.h"


/// @brief /////////////////////
EspSpiBusConfiguratorBuilder::EspSpiBusConfiguratorBuilder()
{
  
  this->xprvBusConfiguration.data1_io_num = NODEF;
  this->xprvBusConfiguration.data2_io_num = NODEF;
  this->xprvBusConfiguration.data3_io_num = NODEF;
  this->xprvBusConfiguration.data4_io_num = NODEF;
  this->xprvBusConfiguration.data5_io_num = NODEF;
  this->xprvBusConfiguration.data6_io_num = NODEF;
  this->xprvBusConfiguration.data7_io_num = NODEF;

}

void *EspSpiBusConfiguratorBuilder::xBuild() 
{
  /*Configure the Bus Interface */
#ifndef configREMOTE
  vSetMosi(PIN_NUM_MOSI);
#endif
  vSetClocK(PIN_NUM_CLK);
  vSetMiso(PIN_NUM_MISO);
  vSetMaxTransfer(NUM_BITS);

  return &xprvBusConfiguration;
}

/// @brief //////////////
EspSpiDeviceBuilder::EspSpiDeviceBuilder()
{
  memset(&xprvDeviceConfiguration, 0x0, sizeof(xprvDeviceConfiguration));
}

void *EspSpiDeviceBuilder::xBuild()
{
#ifndef configREMOTE
  vSetChipSelect(GPIO_NUM_1);
  vSetFlags(SPI_DEVICE_TXBIT_LSBFIRST | SPI_DEVICE_RXBIT_LSBFIRST);
  #endif
    vSetMode(1);
    vSetClockSpeed(1 * 1000 * 1000);
    vSetQueueSize(1);
    vSetCallBackFunction(NULL);

    return &xprvDeviceConfiguration;
}

    /// @brief //////////////////
EspSpiTransactionBuilder::EspSpiTransactionBuilder(uint8_t *puTxBuffer, uint8_t *puRxBuffer)
{
  memset(&xprvSpiTransaction, 0x0, sizeof(xprvSpiTransaction));
  assert(puTxBuffer != NULL);
  assert(puRxBuffer != NULL);

  prvuTxBuffer = puTxBuffer;
  prvuRxBuffer = puRxBuffer;
}

void *EspSpiTransactionBuilder::xBuild()
{
  vsetLength(16);
  vsetReceiveLength(16);
  vsetReceivedBuffer(prvuRxBuffer);
  vsetTransmitBuffer(prvuTxBuffer);

  return &xprvSpiTransaction;
}

/// @brief ////////////////
void EspSpiBuilder::xBuildBusConfigure(SpiBusConfiguratorBuilder Builder)
{
  xprvEspBusConfig =*((spi_bus_config_t *)Builder.xBuild());
}

void EspSpiBuilder::xBuildTransaction(SpiTransactionBuilder Builder)
{
 xprvEspTransaction =*((spi_transaction_t *)Builder.xBuild());
}

void EspSpiBuilder::xBuildDevice(SpiDeviceConfigurationBuilder Builder)
{
  xprvEspDevice = *((spi_device_interface_config_t *)Builder.xBuild());
}

void *EspSpiBuilder::xBuild()
{

  spi_bus_initialize(xprvHost, &xprvEspBusConfig, xprvDmaChannel);
  spi_bus_add_device(xprvHost, &xprvEspDevice, &xprvSpiHandle);
  return new EspSpiDriver(xprvHost, xprvSpiHandle, xprvEspTransaction);
}


EspSpiDriver::EspSpiDriver(spi_host_device_t host, spi_device_handle_t handle,
                           spi_transaction_t transaction)
{


}

void EspSpiDriver::Init()
{}

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
