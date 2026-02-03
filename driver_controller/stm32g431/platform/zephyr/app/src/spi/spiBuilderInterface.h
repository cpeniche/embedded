#pragma once
#include "spiInterface.h"

template <class basetype, class errortype>
class SpiBuilder
{

public:
  virtual ~SpiBuilder() {};
  virtual spiInterface<basetype, errortype> *factoryMethod() = 0;
};

