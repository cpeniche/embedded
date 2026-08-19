#include <zephyr/types.h>
#include "nrfI2c.hpp"
#include "nrfI2c.h"

nrfI2c *nrfI2c::_instance = nullptr;

struct i2c_dt_spec bmp280Device = I2C_DT_SPEC_GET(DT_NODELABEL(bmp280));

nrfI2c *nrfI2c::getInstance(void)
{
  if (_instance == nullptr)
  {
    _instance = new nrfI2c();
  }
  return _instance;
}

int8_t nrfI2c::Write(uint8_t *buffer, size_t len)
{
  error = i2c_write_dt(device, buffer, len);
  return error;
}

int8_t nrfI2c::Read(uint8_t *buffer, size_t len)
{
  error = i2c_read_dt(device, buffer, len);
  return error;
}

void nrfI2c::SetDevice(void *device)
{
  this->device = (struct i2c_dt_spec *)device;
}

nrfI2c::nrfI2c(void)
{
  printk("Create object");
}

extern "C"
{

  nrfI2cCppClass *nrfI2Object = reinterpret_cast<nrfI2cCppClass *>(nrfI2c::getInstance());

  int8_t cppI2cRead(uint8_t *buffer, size_t len)
  {
    (reinterpret_cast<nrfI2c *>(nrfI2Object))->SetDevice((void *)&bmp280Device);
    return (reinterpret_cast<nrfI2c *>(nrfI2Object))->Read(buffer, len);
  }
  int8_t cppI2cWrite(uint8_t *buffer, size_t len)
  {
    (reinterpret_cast<nrfI2c *>(nrfI2Object))->SetDevice((void *)&bmp280Device);
    return (reinterpret_cast<nrfI2c *>(nrfI2Object))->Write(buffer, len);
  }

  int8_t cppIsI2cReady()
  {
    int8_t err = 0;

    (reinterpret_cast<nrfI2c *>(nrfI2Object))->SetDevice((void *)&bmp280Device);
    err = (reinterpret_cast<nrfI2c *>(nrfI2Object))->deviceRedy();

    return !err;
  }

  void zephyrDelay(uint32_t delayMS)
  {
    k_msleep(delayMS);
  }
}