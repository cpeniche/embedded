
#pragma once

#define NODEF -1

/* Virtual Classes*/
class SpiDriver
{

public:
  virtual ~SpiDriver(){};
  virtual SpiDriver *Init() const = 0;
  virtual void Transmit() const = 0;
  virtual uint8_t *GetReceiveData();
};

class SpiBusConfiguratorBuilder{

  public:
    virtual ~SpiBusConfiguratorBuilder() {};
    virtual void xBuild(){};
};

class SpiTransactionBuilder{
  public:
    virtual ~SpiTransactionBuilder() {};
    virtual void xBuild() {};
};

class SpiDeviceConfigurationBuilder{
  public:
    virtual ~SpiDeviceConfigurationBuilder() {};
    virtual void xBuild() {};
    ;
};

class SpiBuilder
{

public:
  virtual ~SpiBuilder() {};
  virtual SpiDriver *xBuild(uint8_t *prvuTxBuffer, uint8_t *prvuRxBuffer) = 0;
  virtual void xBuildBusConfigure(SpiBusConfiguratorBuilder) = 0;
  virtual void xBuildTransaction(SpiTransactionBuilder) = 0;
  virtual void xBuildDevice(SpiDeviceConfigurationBuilder) = 0;

private:
  uint8_t *puTxBuffer;
  uint8_t *puRxBuffer;
};