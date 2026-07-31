#pragma once
#include "busInterfaceBuilder.hpp"
#include "zephyrSpiWrapper.hpp"


class zephyrSpiWrapperBuilder : public busInterfaceBuilder
{

public:
  busInterface *Build() const override {
    return new zephyrSpiWrapper();
  }
};