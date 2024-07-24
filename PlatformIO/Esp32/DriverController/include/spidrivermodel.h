
#pragma once

#define NODEF -1

/* Virtual Classes*/
class SpiDriver
{

public:
  virtual ~SpiDriver(){};
  virtual void Init()  = 0;
  virtual void Transmit()  = 0;
  virtual void *GetReceiveData() = 0;
  virtual void *GetError() = 0;
};

class SpiBusConfiguratorBuilder{

  public:
    virtual ~SpiBusConfiguratorBuilder() {};
    virtual void *xBuild() { return NULL; };
};

class SpiTransactionBuilder{
  public:
    virtual ~SpiTransactionBuilder() {};
    virtual void *xBuild() { return NULL; };
};

class SpiDeviceConfigurationBuilder{
  public:
    virtual ~SpiDeviceConfigurationBuilder() {};
    virtual void *xBuild() { return NULL; };
};

class SpiBuilder{

public:
  virtual ~SpiBuilder() {};  
  virtual void xBuildBusConfigure(SpiBusConfiguratorBuilder) = 0;
  virtual void xBuildTransaction(SpiTransactionBuilder) = 0;
  virtual void xBuildDevice(SpiDeviceConfigurationBuilder) = 0;
  virtual void *xBuild() = 0;
  ;
};