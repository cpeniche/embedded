#pragma once

class motor
{

public:
  virtual ~motor() {};
  virtual void Right(void) = 0;
  virtual void Left(void) = 0;
  virtual void Down(void) = 0;
  virtual void Up(void) = 0;
};