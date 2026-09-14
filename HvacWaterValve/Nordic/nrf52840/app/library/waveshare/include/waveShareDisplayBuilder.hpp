#pragma once
#include "displayInterfaceBuilder.hpp"
#include "waveShareDisplay.hpp"

template <typename typeErr>
class waveShareDisplayBuilder : public displayInterfaceBuilder<typeErr>
{

public:
  displayInterface<typeErr>*Build() const override {
    return new waveShareDisplay<typeErr>();
  }
};