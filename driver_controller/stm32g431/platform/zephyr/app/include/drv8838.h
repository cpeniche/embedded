#pragma once

class drv8838: public motorInterface
{

public:
  drv8838();
  ~drv8838(){};
  void Right(void) override;
  void Left(void) override;
  void Down(void) override;
  void Up(void) override;
  void Idle(void) override;
  
};