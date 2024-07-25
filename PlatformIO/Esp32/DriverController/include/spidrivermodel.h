
#pragma once

#define NODEF -1

/* Virtual Classes*/
class SpiDriver
{

public:
  virtual ~SpiDriver(){};
  virtual void Init()  = 0;
  virtual void Transmit()  = 0;
  virtual void *GetReceiveData() = 0;
  virtual void *GetError() = 0;
};

class SpiBuilder{
public:
  virtual ~SpiBuilder() {};  
  virtual void *xBuild() = 0;
  ;
};