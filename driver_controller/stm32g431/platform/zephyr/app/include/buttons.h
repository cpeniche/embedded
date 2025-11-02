#pragma once

//byte 0
#define maskRIGTHWINDOWUP          0x10
#define maskRIGTHWINDOWDOWN        0x08
#define maskRIGTHWINDOWDOWNCONT    0x28
#define maskLEFTWINDOWUP          0x02
#define maskLEFTWINDOWDOWN        0x01
#define maskLEFTWINDOWDOWNCONT    0x05

//byte 1
#define maskMIRRORSELECTRIGHT  0x80
#define maskMIRRORSELECTLEFT   0x40

//byte 2
#define maskMIRRORUP        0x01 
#define maskMIRRORDOWN      0x02
#define maskMIRRORLEFT      0x04 
#define maskMIRRORRIGHT     0x08
#define maskLOCK            0x40 
#define maskUNLOCK          0x80 

typedef void (motorInterface::*motor_func_t)(void);

struct Actions
  { 
    uint8_t mask;
    motor_func_t motor;    
  };

struct data
  {
    uint8_t u8PrevState;
    uint8_t u8BufferIndex;
    motorInterface *motorClassPtr;
    struct Actions stActions[8];    
  };

class Buttons{

public:
    Buttons();
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
  motorInterface *lock = latchMotorBuilder.factoryMethod();

  /*window motor interface*/
  static vnh5019aMotorBuilder windowMotorBuilder;
  motorInterface *window = windowMotorBuilder.factoryMethod();   

  struct data stWindow =
  {
    0x0,
    0x0,
    window,
    {
      {maskLEFTWINDOWUP, &motorInterface::Up},
      {maskLEFTWINDOWDOWN, &motorInterface::Down},
      {maskLEFTWINDOWDOWNCONT, &motorInterface::Down},
      {maskRIGTHWINDOWUP, &motorInterface::Up},
      {maskRIGTHWINDOWDOWN, &motorInterface::Down},
      {maskRIGTHWINDOWDOWNCONT, &motorInterface::Down},
      {0x0,&motorInterface::Idle},
      {0x0,nullptr}
    }
  };

  struct data stMirror =
  {
    0x0,
    0x2,
    mirror,
    {
      {maskMIRRORUP, &motorInterface::Up},
      {maskMIRRORDOWN, &motorInterface::Down},
      {maskMIRRORLEFT, &motorInterface::Left},
      {maskMIRRORRIGHT, &motorInterface::Right},    
      {0x0,&motorInterface::Idle},
      {0x0,nullptr}
    }
  };

  struct data stLock =
  {
    0x0,
    0x2,
    lock,
    {      
      {maskLOCK, &motorInterface::Up},
      {maskUNLOCK, &motorInterface::Down},
      {0x0,&motorInterface::Idle},
      {0x0,nullptr}
    }
  };

};