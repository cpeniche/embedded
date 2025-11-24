#pragma once
#include "inputInterface.h"
#include "adc.h"
#include "lin.h"

class inputBuilder
{
public:
  virtual ~inputBuilder() {};
  virtual inputInterface *factoryMethod(struct device *, uint8_t *, size_t, uint8_t, uart_callback_t) = 0;
};

class linInputBuilder : public inputBuilder
{

public:
  inputInterface *factoryMethod(struct device *device, uint8_t *rxBuffer, size_t size, uint8_t length, uart_callback_t func) override
  {
    Driver = new LIN(device, rxBuffer, size, length);
    Driver->setCallback(func);
    return Driver;
  }
  LIN *getLinDriver() { return Driver; }

private:
  LIN *Driver;
};

class adcInputBuilder : public inputBuilder
{

public:
  inputInterface *factoryMethod(struct device *device, uint8_t *rxBuffer, size_t size, uint8_t length, uart_callback_t func) override
  {
    return new adc();
  }
};
