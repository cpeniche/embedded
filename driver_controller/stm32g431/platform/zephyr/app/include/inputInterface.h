#pragma once

class inputInterface
{

public:
  virtual ~inputInterface() {};

  virtual int8_t readInput(uint8_t *, size_t) = 0;
  virtual int8_t getInput(uint8_t *buffer) = 0;
  virtual bool isDataReady(void) = 0;
  virtual int8_t getError(void) = 0;
};