
#pragma once

#define DUAL 0
#define QUAD 1
#define OCTAL 2
#define NODEF -1

/* Virtual Classes*/
class SpiDriver
{

public:
  virtual ~SpiDriver(){};
  virtual void Read() = 0;
  virtual void Write() = 0;
};

class SpiCreator{

public:
  virtual ~SpiCreator() {};
  virtual SpiDriver *CreateSpi() const = 0;
};

/* Concrete Classes*/
class EspSpi : public SpiDriver
{
public:
  EspSpi() {};
  void Read() const override {

  };
  void Write() const override {

  };
};



class EspSpiCreator : public SpiCreator
{
public:
  SpiDriver *CreateSpi() const override
  {
    return new EspSpi();
  }

}


template <class type>
class SpiBusConfigBuilder
{

public:
  SpiBusConfigBuilder(uint8_t uprvInterfaceSize)
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

  type vBuild()
  {
    assert(xprvBusConfiguration.sclk_io_num != NODEF);
    return xprvBusConfiguration = ;
  };

private:
  type xprvBusConfiguration;
  uint8_t uprvInterfaceSize;
};

template <class type>
class SpiDeviceBuilder
{

public:
  
  SpiDeviceBuilder() { 
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

  type vBuild()
  {
    return xprvDeviceConfiguration;
  }

private:
  type xprvDeviceConfiguration;
};