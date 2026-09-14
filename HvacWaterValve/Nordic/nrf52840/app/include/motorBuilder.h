#pragma once
#include "motorInterface.h"
#include "drv8838.h"


class motorBuilder{
  public:
    virtual ~motorBuilder(){};
    virtual motorInterface *factoryMethod(void)=0;

};

class drv8838MotorBuilder : public motorBuilder
{
  public:
    motorInterface *factoryMethod(void) override{     
      return new drv8838();
  }

};