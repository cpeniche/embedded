#pragma once

#include <string.h>
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
//#include "motors.h"
//#include "main.h"
#include "spidrivermodel.h"

/* Concrete Classes for generating ESP definitions*/

class EspSpiBusConfiguratorBuilder : public SpiBusConfiguratorBuilder
{
  public : 
    EspSpiBusConfiguratorBuilder();


    void vSetMiso(int iMiso) { xprvBusConfiguration.miso_io_num = iMiso; };
    void vSetMosi(int iMosi) { xprvBusConfiguration.mosi_io_num = iMosi; };
    void vSetClocK(int iClock) { xprvBusConfiguration.sclk_io_num = iClock; };
    void vSetMaxTransfer(int iMaxTransfer) { xprvBusConfiguration.max_transfer_sz = iMaxTransfer; };
    void vSetFlags(uint32_t uFlags) { xprvBusConfiguration.flags = uFlags; };
    void vSetInterruptCPUAffinity(esp_intr_cpu_affinity_t xprvAffinity) { xprvBusConfiguration.isr_cpu_id = xprvAffinity; };
    void vSetInterruptFlags(int8_t iprvInterruptFlags) { xprvBusConfiguration.intr_flags = iprvInterruptFlags; };

    void vSetData0(int iData0) { xprvBusConfiguration.data0_io_num = iData0; };
    void vSetData1(int iData1) { xprvBusConfiguration.data1_io_num = iData1; };
    void vSetData2(int iData2) { xprvBusConfiguration.data2_io_num = iData2; };
    void vSetData3(int iData3) { xprvBusConfiguration.data3_io_num = iData3; };
    void vSetData4(int iData4) { xprvBusConfiguration.data4_io_num = iData4; };
    void vSetData5(int iData5) { xprvBusConfiguration.data5_io_num = iData5; };
    void vSetData6(int iData6) { xprvBusConfiguration.data6_io_num = iData6; };
    void vSetData7(int iData7) { xprvBusConfiguration.data7_io_num = iData7; };

    void *xBuild() override;

  private:
    spi_bus_config_t xprvBusConfiguration;
};


class EspSpiDeviceBuilder:public SpiDeviceConfigurationBuilder
{

public:
  EspSpiDeviceBuilder();

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

  void *xBuild() override;

private:
  spi_device_interface_config_t xprvDeviceConfiguration;
};



class EspSpiTransactionBuilder: public SpiTransactionBuilder{

  public:
    EspSpiTransactionBuilder(uint8_t *puTxBuffer, uint8_t *puRxBuffer);
    void vsetFlags(uint32_t uFlags) { xprvSpiTransaction.flags = uFlags; };
    void vsetCommand(uint16_t uCommand) { xprvSpiTransaction.cmd = uCommand; };
    void vsetAddress(uint64_t uAddress) { xprvSpiTransaction.addr = uAddress; };
    void vsetLength(size_t xLength) { xprvSpiTransaction.length = xLength; };
    void vsetReceiveLength(size_t xReciveLength) { xprvSpiTransaction.rxlength = xReciveLength; };
    void vsetUser(void* pvUser) { xprvSpiTransaction.user = pvUser; };
    void vsetTransmitBuffer(void* pvTransmitBuffer) { xprvSpiTransaction.tx_buffer = pvTransmitBuffer; };
    void vsetReceivedBuffer(void* pvReceiveBuffer) { xprvSpiTransaction.rx_buffer = pvReceiveBuffer; };

    void *xBuild() override;

  private:
    spi_transaction_t xprvSpiTransaction;
    uint8_t *prvuTxBuffer;
    uint8_t *prvuRxBuffer;
};

class EspSpiDriver : public SpiDriver
{
  void Init() const = 0;
  void Transmit() const = 0;
  uint8_t *GetReceiveData();
};


class EspSpiBuilder : public SpiBuilder
{
public:
  EspSpiBuilder(){};

  //EspSpiDriver *xBuild(){};
  void xBuildBusConfigure(SpiBusConfiguratorBuilder) override;
  void xBuildTransaction(SpiTransactionBuilder) override;
  void xBuildDevice(SpiDeviceConfigurationBuilder) override;

  void *pvGetBusConfiguration() { return &xprvEspBusConfig; };
  void *pvGetDevice() { return &xprvEspDevice; };
  void *pvGetTransaction() { return &xprvEspTransaction; }

private:
  spi_bus_config_t xprvEspBusConfig;
  spi_device_interface_config_t xprvEspDevice;
  spi_transaction_t xprvEspTransaction;
  spi_device_handle_t xprvSpiHandle;
};


