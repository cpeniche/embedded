#pragma once
#include "inputInterface.h"


class inputBuilder
{
public:
  virtual ~inputBuilder() {};
  virtual inputInterface *factoryMethod(struct device *, uint8_t *, size_t, uint8_t, uart_callback_t) = 0;
};