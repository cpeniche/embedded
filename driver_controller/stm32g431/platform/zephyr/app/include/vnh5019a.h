#pragma once

class vnh5019a: public motorInterface
{

public:
  vnh5019a();
  ~vnh5019a(){};
  void Right(void) override;
  void Left(void) override;
  void Down(void) override;
  void Up(void) override;
  void Idle(void) override;
  
};