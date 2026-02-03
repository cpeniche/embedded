#pragma once
#include "spiBuilderInterface.h"
#include "zephyrSpi.h"

template <class basetype, class errortype>
class zephyrSpiBuilder : public SpiBuilder<basetype, errortype>
{

public:
  spiInterface<basetype, errortype> *factoryMethod() override
  {
    //return new zephyrSpi<basetype, errortype>(config);
    return zephyrSpi<basetype, errortype>::getInstance();
  }
};