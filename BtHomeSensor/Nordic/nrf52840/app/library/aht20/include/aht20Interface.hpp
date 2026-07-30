#pragma once

class aht20Interface
{

public:
  virtual int8_t Init() = 0;
  virtual uint32_t ReadHumidity(void) = 0;
  virtual uint32_t ReadTemperature(void) = 0;
  virtual int8_t TriggerMeasurement(void (*delayFunc)(uint32_t), uint32_t ms) = 0;
  ;
};