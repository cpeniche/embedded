#pragma once
#include "motorInterface.h"
#include "tle94103.h"
#include "drv8838.h"
#include "vnh5019a.h"

class motorBuilder{
  public:
    virtual ~motorBuilder(){};
    virtual motorInterface *factoryMethod()=0;

};

class tle94103MotorBuilder : public motorBuilder
{
  public:
    motorInterface *factoryMethod() override{
      return new Tle94103();
  }

};

class drv8838MotorBuilder : public motorBuilder
{
  public:
    motorInterface *factoryMethod() override{
      return new drv8838();
  }

};

class vnh5019aMotorBuilder : public motorBuilder
{
  public:
    motorInterface *factoryMethod() override{
      return new vnh5019a();
  }

};