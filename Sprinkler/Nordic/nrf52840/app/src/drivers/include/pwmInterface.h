#pragma once

class pwmInterface {

public:
  virtual int8_t setDutyCycle(uint8_t ) = 0;
  virtual int8_t setFrequency(uint32_t ) = 0;
  virtual int8_t start() = 0;
  virtual int8_t stop() = 0;

};