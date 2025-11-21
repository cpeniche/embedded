#pragma once

class inputInterface
{

public:
  virtual ~inputInterface() {};

  virtual int8_t readInput(uint8_t *, size_t);
  virtual uint8_t *getInput(void);
  virtual bool getDataReady(void);
};