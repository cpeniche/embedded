#pragma once
#include <zephyr/drivers/i2c.h>
#include "i2cInterface.hpp"

class nrfI2c : public i2cInterface
{
public:
  nrfI2c(nrfI2c &) = delete;
  void operator=(const nrfI2c &) = delete;
  static nrfI2c *getInstance(void);
  int8_t Write(uint8_t *buffer, size_t len) override;
  int8_t Read(uint8_t *buffer, size_t len) override;
  void SetDevice(void *device) override;
  bool deviceRedy() { return i2c_is_ready_dt(device); }

private:
  struct i2c_dt_spec *device;

protected:
  nrfI2c(void);
  static nrfI2c *_instance;
  int8_t error=0;
};
