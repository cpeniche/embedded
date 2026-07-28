
#include "bmp280.h"

bmp280::bmp280()
{
  _interface = nrfI2c::getInstance();
}

int8_t bmp280::configure()
{
  
  return 0;
}

int8_t bmp280::ReadPressure()
{
  return 0;
}

int8_t bmp280::ReadTemperature()
{
  return 0;
}

int8_t bmp280::ReadDeviceCode(uint8_t *data)
{
  uint8_t buffer=idReg;
  int8_t err;
  
  err = _interface->Write(&buffer,1);
  if (err<0) return err;
  err = _interface->Read(&buffer, 1);
  if (err<0) return err;
  
  *data=buffer;
  return 0;
}

