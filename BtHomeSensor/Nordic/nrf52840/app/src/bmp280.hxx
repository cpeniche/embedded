#pragma once
#include <zephyr/types.h>
#include "nrfI2c.h"


#define idReg 0xD0

class bmp280{

public:
  bmp280();
  int8_t configure();
  int8_t ReadPressure();
  int8_t ReadTemperature();
  int8_t ReadDeviceCode(uint8_t*);

private:
  uint32_t _temperature;
  uint32_t _pressure;
  nrfI2c *_interface=nullptr;
  
};