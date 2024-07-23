
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
  virtual void Transmit() const = 0;
};

class SpiCreator{

public:
  virtual ~SpiCreator() {};
  virtual SpiDriver *Create() = 0;
};

template <class type>
class SpiBusConfiguratorBuilder{

  public:
    virtual ~SpiBusConfiguratorBuilder() {};
    virtual type xBuild() const = 0;
};

template <class type>
class SpiTransactionBuilder{
  public:
    virtual ~SpiTransactionBuilder() {};
    virtual type xBuild() const = 0;
};

template <class type>
class SpiDeviceConfigurationBuilder{
  public:
    virtual ~SpiDeviceConfigurationBuilder() {};
    virtual type xBuild() const = 0;
};
