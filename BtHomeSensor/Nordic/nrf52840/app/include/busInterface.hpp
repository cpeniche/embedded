#pragma once
#include <stdint.h>
#include <stddef.h>

class busInterface
{

public:
  virtual int8_t Write(uint8_t *buffer, size_t len) = 0;
  virtual int8_t Read(uint8_t *buffer, size_t len) = 0;
  virtual int8_t WriteRead(uint8_t *, size_t , uint8_t *, size_t ) = 0;
  virtual void SetDevice(void *)=0;
};