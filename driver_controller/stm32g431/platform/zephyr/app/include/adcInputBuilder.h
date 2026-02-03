#pragma once 
#include "inputInterface.h"
#include "inputBuilder.h"
#include "adc.h"

class adcInputBuilder : public inputBuilder
{

public:
  inputInterface *factoryMethod(struct device *device, uint8_t *rxBuffer, size_t size, uint8_t length, uart_callback_t func) override
  {
    return new adc();
    }
};