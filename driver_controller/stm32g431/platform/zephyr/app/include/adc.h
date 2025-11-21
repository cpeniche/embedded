#pragma once

class adc : public inputInterface
{

public:
  adc();
  ~adc() {};
  int8_t getInput(uint8_t *, size_t) override;
};