#pragma once
//#include "spiInterface.h"
#include "zephyrSpi.h"

template <class basetype, class errortype>
class SpiBuilder
{

public:
  virtual ~SpiBuilder() {};
  virtual spiInterface<basetype, errortype> *factoryMethod() = 0;  
};

template <class basetype, class errortype>
class zephyrSpiBuilder : public SpiBuilder<basetype,errortype>
{

public:
  spiInterface<basetype, errortype> *factoryMethod() override
  {
    return new zephyrSpi<basetype, errortype>();
  }
};