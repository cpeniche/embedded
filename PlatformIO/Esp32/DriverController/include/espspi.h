#pragma once

#include <string.h>
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "motors.h"
#include "main.h"
#include "spi.h"

template <class type>
class EspSpiBusConfiguratorBuilder: public SpiBusConfiguratorBuilder<type>
{

public:
  EspSpiBusConfiguratorBuilder(uint8_t uprvInterfaceSize)
  {
    this->uprvInterfaceSize = uprvInterfaceSize;
    this->xprvBusConfiguration.data1_io_num = NODEF;
    this->xprvBusConfiguration.data2_io_num = NODEF;
    this->xprvBusConfiguration.data3_io_num = NODEF;
    this->xprvBusConfiguration.data4_io_num = NODEF;
    this->xprvBusConfiguration.data5_io_num = NODEF;
    this->xprvBusConfiguration.data6_io_num = NODEF;
    this->xprvBusConfiguration.data7_io_num = NODEF;
  };

  void vSetMiso(int iMiso) { xprvBusConfiguration.miso_io_num = iMiso; };
  void vSetMosi(int iMosi) { xprvBusConfiguration.mosi_io_num = iMosi; };
  void vSetClocK(int iClock) { xprvBusConfiguration.sclk_io_num = iClock; };
  void vSetMaxTransfer(int iMaxTransfer) { xprvBusConfiguration.max_transfer_sz = iMaxTransfer; };
  void vSetFlags(uint32_t uFlags) { xprvBusConfiguration.flags = uFlags; };
  void vSetInterruptCPUAffinity(esp_intr_cpu_affinity_t xprvAffinity) { xprvBusConfiguration.isr_cpu_id = xprvAffinity; };
  void vSetInterruptFlags(int8_t iprvInterruptFlags) { xprvBusConfiguration.intr_flags = iprvInterruptFlags; };

  void vSetData0(int iData0) { xprvBusConfiguration.data1_io_num = iData0; };
  void vSetData1(int iData1) { xprvBusConfiguration.data2_io_num = iData1; };
  void vSetData2(int iData2) { xprvBusConfiguration.data3_io_num = iData2; };
  void vSetData3(int iData3) { xprvBusConfiguration.data4_io_num = iData3; };
  void vSetData4(int iData4) { xprvBusConfiguration.data5_io_num = iData4; };
  void vSetData5(int iData5) { xprvBusConfiguration.data6_io_num = iData5; };
  void vSetData6(int iData6) { xprvBusConfiguration.data7_io_num = iData6; };
  void vSetData7(int iData7) { xprvBusConfiguration.data8_io_num = iData7; };

  type xBuild() const override
  {
    assert(xprvBusConfiguration.sclk_io_num != NODEF);
    return xprvBusConfiguration;
  };

private:
  type xprvBusConfiguration;
  uint8_t uprvInterfaceSize;
};

template <class type>
class EspSpiDeviceBuilder:public SpiDeviceConfigurationBuilder<type>
{

public:
  
  EspSpiDeviceBuilder() { 
    memset(&xprvDeviceConfiguration,0x0,sizeof(xprvDeviceConfiguration));
  };

  void vSetCommandBits(uint8_t uCommandBits) { xprvDeviceConfiguration.command_bits = uCommandBits; };
  void vSetAddresBits(uint8_t uAddressBits) { xprvDeviceConfiguration.address_bits = uAddressBits; };
  void vSetMode(uint8_t uMode) { xprvDeviceConfiguration.mode = uMode; };
  void vSetClockSource(spi_clock_source_t xClockSource) { xprvDeviceConfiguration.clock_source = xClockSource; };
  void vSetDutyCycle(uint16_t uDutyCycle) { xprvDeviceConfiguration.duty_cycle_pos = uDutyCycle; };
  void vSetChipEnablePretransfer(uint16_t uChipEnablePreTransfer) { xprvDeviceConfiguration.cs_ena_pretrans = uChipEnablePreTransfer; };
  void vSetChipEnablePostTransfer(uint16_t uChipEnablePostTransfer) { xprvDeviceConfiguration.cs_ena_posttrans = uChipEnablePostTransfer; };
  void vSetClockSpeed(int iClockSpeed) { xprvDeviceConfiguration.clock_speed_hz = iClockSpeed; };
  void vSetInputDelay(int iInputDelayNs) { xprvDeviceConfiguration.input_delay_ns = iInputDelayNs; };
  void vSetChipSelect(int iChipSelect) { xprvDeviceConfiguration.spics_io_num = iChipSelect; };
  void vSetFlags(uint32_t uFlags) { xprvDeviceConfiguration.flags = uFlags; };
  void vSetQueueSize(int uQueueSize) { xprvDeviceConfiguration.queue_size = uQueueSize; };
  void vSetCallBackFunction(transaction_cb_t xCallBackFunction) { xprvDeviceConfiguration.pre_cb = xCallBackFunction; };

  type xBuild() const override
  {
    return xprvDeviceConfiguration;
  }

private:
  type xprvDeviceConfiguration;
};


template <class type>
class EspSpiTransactionBuilder: SpiTransactionBuilder<type>{

  public:
  EspSpiTransactionBuilder(){
    memset(&xprvSpiTransaction, 0x0, sizeof(xprvSpiTransaction));
  }
    void vsetFlags(uint32_t uFlags) { xprvSpiTransaction.flags = uFlags; };
    void vsetCommand(uint16_t uCommand) { xprvSpiTransaction.cmd = uCommand; };
    void vsetAddress(uint64_t uAddress) { xprvSpiTransaction.addr = uAddress; };
    void vsetLength(size_t xLength) { xprvSpiTransaction.length = xLength; };
    void vsetReceiveLength(size_t xReciveLength) { xprvSpiTransaction.rxlength = xReciveLength; };
    void vsetUser(void* pvUser) { xprvSpiTransaction.user = pvUser; };
    void vsetTransmitBuffer(void* pvTransmitBuffer) { xprvSpiTransaction.tx_buffer = pvTransmitBuffer; };
    void vsetReceivedBuffer(void* pvReceiveBuffer) { xprvSpiTransaction.rx_buffer = pvReceiveBuffer; };

    type xBuild() const override
    {
      return xprvSpiTransaction;
    };

  private:
    type xprvSpiTransaction;
};

/* Concrete Classes*/
template <class HandleType>
class EspSpi : public SpiDriver
{
public:
  EspSpi(HandleType xprvHandle) 
  {
    xprvSpiHandle = xprvHandle;
  };
  
  void Transmit() const override {

  };

private:
  HandleType xprvSpiHandle;
};

class EspSpiCreator : public SpiCreator
{
public:
  SpiDriver *Create() override
  {

    xprvConfig = new EspSpiBusConfiguratorBuilder<spi_bus_config_t>(DUAL);

#ifndef configREMOTE
  xprvConfig->vSetMosi(PIN_NUM_MOSI);
#endif
  xprvConfig->vSetClocK(PIN_NUM_CLK);
  xprvConfig->vSetMiso(PIN_NUM_MISO);
  xprvConfig->vSetMaxTransfer(NUM_BITS);


  xprvDevice = new EspSpiDeviceBuilder<spi_device_interface_config_t>;
#ifndef configREMOTE
  xprvDevice->vSetChipSelect(TLEMOTORCHIPSELECT);
  xprvDevice->vSetFlags(SPI_DEVICE_TXBIT_LSBFIRST | SPI_DEVICE_RXBIT_LSBFIRST);
#endif
  xprvDevice->vSetMode(1);
  xprvDevice->vSetClockSpeed(1 * 1000 * 1000);
  xprvDevice->vSetQueueSize(1);
  xprvDevice->vSetCallBackFunction(NULL);

  //xprvTransaction = new EspSpiTransactionBuilder<spi_transaction_t>;

  //spi_bus_initialize(SPI2_HOST, &xprvConfig->xBuild(), SPI_DMA_CH_AUTO);
  //spi_bus_add_device((SPI2_HOST, &xprvDevice->xBuild(), &xprvSpiHandle));

  return new EspSpi<spi_device_handle_t>(xprvSpiHandle);
  }

private:
  EspSpiBusConfiguratorBuilder<spi_bus_config_t> *xprvConfig;
  EspSpiDeviceBuilder<spi_device_interface_config_t> *xprvDevice;
  spi_device_handle_t xprvSpiHandle;
};

