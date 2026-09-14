#pragma once

#include "nrfI2c.hpp"

class i2cFactory
{

public:
  virtual ~i2cFactory() {};
  virtual i2cInterface *Factory() const = 0;
};

class i2cNrfI2cFactory : public i2cFactory
{

public:
  i2cInterface *Factory() const override
  {
    return nrfI2c::getInstance();
  }
};