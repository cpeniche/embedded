#pragma once
#include "spiInterface.h"
#include "zephyrSpi.h"

template <class basetype, class errortype>
class SpiBuilder
{

public:
  virtual ~SpiBuilder() {};
  virtual spiInterface<basetype, errortype> *createSpi() const = 0;
};

template <class basetype, class errortype>
class zephyrSpiBuilder : public SpiBuilder<basetype,errortype>
{

public:
  zephyrSpi<basetype, errortype> *createSpi() const override
  {
    return new zephyrSpi<basetype, errortype>();
  }
};