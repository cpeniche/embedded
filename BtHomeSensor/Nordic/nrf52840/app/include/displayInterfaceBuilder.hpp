#pragma once
#include "displayInterface.hpp"

template <typename typeErr>
class displayInterfaceBuilder{

public:
  //virtual ~displayInterfaceBuilder<typeErr>(){};
  virtual displayInterface<typeErr>*Build() const =0;
};