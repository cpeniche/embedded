#pragma once
#include "spiBuilderInterface.h"
#include "zephyrSpi.h"

/**
 * @brief 
 * 
 * @tparam basetype 
 * @tparam errortype 
 */
template <class basetype, class errortype>
class zephyrSpiBuilder : public SpiBuilder<basetype, errortype>
{

public:
  spiInterface<basetype, errortype> *factoryMethod() override
  {
    return zephyrSpi<basetype, errortype>::getInstance();
  }
};