#include <zephyr/types.h>
#include "nrfI2c.h"

nrfI2c *nrfI2c::_instance = nullptr;

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
  return i2c_write_dt(&device,buffer,len);
}

int8_t nrfI2c::Read(uint8_t *buffer, size_t len)
{
  return i2c_read_dt(&device,buffer,len);  
}

nrfI2c::nrfI2c(void)
{
 if(!i2c_is_ready_dt(&device))
  {
    printk("I2C device not ready\n");
  }
}
