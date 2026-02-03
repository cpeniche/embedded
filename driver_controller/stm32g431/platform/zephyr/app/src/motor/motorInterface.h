#pragma once

class motorInterface
{

public:
  virtual ~motorInterface() {};
  virtual void Idle(uint8_t) = 0;
  virtual void Right(uint8_t) = 0;
  virtual void Left(uint8_t) = 0;
  virtual void Down(uint8_t) = 0;
  virtual void Up(uint8_t) = 0;
};