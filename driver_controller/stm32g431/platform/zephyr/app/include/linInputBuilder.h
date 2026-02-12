
#pragma once
#include "inputInterface.h"
#include "inputBuilder.h"
#include "lin.h"
class linInputBuilder : public inputBuilder
{

public:
  inputInterface *factoryMethod(struct device *device, uint8_t *rxBuffer, size_t size, uint8_t length, uart_callback_t func) override
  {
    Driver = new LIN(device, rxBuffer, size, length, func);

    if(Driver->enableReceive() != 0)
      return nullptr;
    else    
      return Driver;
  }
  LIN *getLinDriver() { return Driver; }

private:
  LIN *Driver;
};
