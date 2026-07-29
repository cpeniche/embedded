#pragma once

class aht20Interface{

public:
  virtual int8_t Init()=0;
  virtual int8_t ReadHumidity(void *)=0;
  virtual int8_t ReadTemperature(void *)=0;

};