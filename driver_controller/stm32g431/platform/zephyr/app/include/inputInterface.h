#pragma once

class inputInterface
{

public:
  virtual ~inputInterface() {};

  virtual int8_t readInput(uint8_t *, size_t) = 0;
  virtual uint8_t *getInput(void) = 0;
  virtual bool getDataReady(void) = 0;
};