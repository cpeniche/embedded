#pragma once

class drv8838: public motorInterface
{

public:
  drv8838(can *);
  ~drv8838(){};
  void Right(uint8_t) override;
  void Left(uint8_t) override;
  void Down(uint8_t) override;
  void Up(uint8_t) override;
  void Idle(uint8_t) override;
  void setCanDriver(can *driver){CanDriver=driver;}

private:

  can *CanDriver;
  uint8_t canTxBufer[2];
};