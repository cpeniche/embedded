#pragma once
#include "busInterface.hpp"

class busInterfaceBuilder
{

public:
  virtual ~busInterfaceBuilder() {};
  virtual busInterface *Build(void) const = 0;
};
