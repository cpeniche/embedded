#pragma once

template <typename typeErr>
class displayInterface{

public:
  virtual ~displayInterface()=default;
  virtual typeErr SetInterface(busInterface *)=0;
  virtual typeErr Init(void) = 0;
  virtual typeErr Update(void)=0;
  virtual typeErr PowerOn(void)=0;
  virtual typeErr PowerOff(void)=0;
  virtual typeErr ClearScreen(void)=0;

};

