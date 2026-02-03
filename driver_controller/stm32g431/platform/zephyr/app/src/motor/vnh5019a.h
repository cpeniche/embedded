#pragma once

class vnh5019a: public motorInterface
{

public:
  vnh5019a(can *);
  ~vnh5019a(){};
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