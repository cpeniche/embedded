#pragma once

// byte 0
#define maskRIGTHWINDOWUP 0x10
#define maskRIGTHWINDOWDOWN 0x08
#define maskRIGTHWINDOWDOWNCONT 0x28
#define maskLEFTWINDOWUP 0x02
#define maskLEFTWINDOWDOWN 0x01
#define maskLEFTWINDOWDOWNCONT 0x05

// byte 1
#define maskMIRRORSELECTRIGHT 0x80
#define maskMIRRORSELECTLEFT 0x40

// byte 2
#define maskMIRRORUP 0x01
#define maskMIRRORDOWN 0x02
#define maskMIRRORLEFT 0x04
#define maskMIRRORRIGHT 0x08
#define maskLOCK 0x40
#define maskUNLOCK 0x80

#define LEFT 0x0
#define RIGHT 0x1

struct device *device = (struct device *)DEVICE_DT_GET(DT_NODELABEL(usart1));

typedef void (motorInterface::*motor_func_t)(uint8_t);

struct Actions
{
  uint8_t mask;
  uint8_t *side;
  motor_func_t motor;
};

struct data
{
  uint8_t u8PrevState;
  uint8_t u8BufferIndex;
  motorInterface *motorClassPtr;
  struct Actions stActions[8];
};

class Buttons
{

public:
  Buttons() {};
  ~Buttons() {}
  LIN *getDriver() { return linButtonsReader.getLinDriver(); }
  void Task(void);

private:
  uint8_t rxBuffer[7] = {0};
  /* mirror motor interface*/
  tle94103MotorBuilder mirrorMotorBuilder;
  motorInterface *mirror = mirrorMotorBuilder.factoryMethod(&CanDriver);
  /* latch motor interface*/
  drv8838MotorBuilder latchMotorBuilder;
  motorInterface *lock = latchMotorBuilder.factoryMethod(&CanDriver);
  /*window motor interface*/
  static vnh5019aMotorBuilder windowMotorBuilder;
  motorInterface *window = windowMotorBuilder.factoryMethod(&CanDriver);

  linInputBuilder linButtonsReader;
  adcInputBuilder adcButtonsReader;
  
  can CanDriver;

  uint8_t u8RightWindow = RIGHT;
  uint8_t u8LeftWindow = LEFT;
  uint8_t u8Mirror = LEFT;

  struct data stWindow =
      {
          0x0,
          0x0,
          window,
          {{maskLEFTWINDOWUP, &u8LeftWindow, &motorInterface::Up},
           {maskLEFTWINDOWDOWN, &u8LeftWindow, &motorInterface::Down},
           {maskLEFTWINDOWDOWNCONT, &u8LeftWindow, &motorInterface::Down},
           {maskRIGTHWINDOWUP, &u8RightWindow, &motorInterface::Up},
           {maskRIGTHWINDOWDOWN, &u8RightWindow, &motorInterface::Down},
           {maskRIGTHWINDOWDOWNCONT, &u8RightWindow, &motorInterface::Down},
           {0x0, nullptr, &motorInterface::Idle},
           {0x0, nullptr, nullptr}}};

  struct data stMirror =
      {
          0x0,
          0x2,
          mirror,
          {{maskMIRRORUP, &u8Mirror, &motorInterface::Up},
           {maskMIRRORDOWN, &u8Mirror, &motorInterface::Down},
           {maskMIRRORLEFT, &u8Mirror, &motorInterface::Left},
           {maskMIRRORRIGHT, &u8Mirror, &motorInterface::Right},
           {0x0, nullptr, &motorInterface::Idle},
           {0x0, nullptr, nullptr}}};

  struct data stLock =
      {
          0x0,
          0x2,
          lock,
          {{maskLOCK, nullptr, &motorInterface::Up},
           {maskUNLOCK, nullptr, &motorInterface::Down},
           {0x0, nullptr, &motorInterface::Idle},
           {0x0, nullptr, nullptr}}};
};