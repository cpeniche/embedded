#pragma once

class i2cInterface{
  
public:
  virtual int8_t Write(uint8_t* buffer, size_t len) = 0;
  virtual int8_t Read(uint8_t* buffer, size_t len) = 0;
};