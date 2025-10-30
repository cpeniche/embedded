#pragma once

class motorInterface
{

public:
  virtual ~motorInterface() {};
  virtual void Idle(void) = 0;
  virtual void Right(void) = 0;
  virtual void Left(void) = 0;
  virtual void Down(void) = 0;
  virtual void Up(void) = 0;
};