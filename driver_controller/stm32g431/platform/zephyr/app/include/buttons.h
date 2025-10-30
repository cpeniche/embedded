#pragma once

extern "C" struct k_queue *getQueue(void);

class Buttons{

public:
    Buttons(){};
    ~Buttons(){}
    int8_t Read(uint8_t *, size_t);
    void Task(void);           
    void SetDriver(LIN *driver){Driver=driver;}
    uint8_t *getData(){return Driver->getRxBuffer();}
    bool getDataReady();
      
private:  
  LIN *Driver;
  /* mirror motor interface*/
  tle94103MotorBuilder mirrorMotorBuilder;
  motorInterface *mirror = mirrorMotorBuilder.factoryMethod();
  /* latch motor interface*/
  drv8838MotorBuilder latchMotorBuilder;
  motorInterface *latch = latchMotorBuilder.factoryMethod();
  /*window motor interface*/
  vnh5019aMotorBuilder windowMotorBuilder;
  motorInterface *window = windowMotorBuilder.factoryMethod();
};