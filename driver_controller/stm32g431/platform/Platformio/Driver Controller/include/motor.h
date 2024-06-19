
#ifndef _motor_h_
#define _motor_h_

#include <stdint.h>
#include "dictionary.h"
#include "halIncludes.h"

#define xSetPinState HAL_GPIO_WritePin
#define xPwmStart    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1)  
#define xPwmStop     HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_1)

#define QUEUE_LENGTH  5
#define ITEM_SIZE     sizeof(eLATCHPOSITION)

enum eMOTORDIRECTION
{
  eSTOP,
  eMOVEUP,
  eMOVEDOWN
};

enum eLATCHPOSITION
{
  eOPEN,
  eCLOSE
};

extern void vMotorCallback(eReadWrite mode);
extern void vDoorLatchCallback(eReadWrite mode);
extern void Motor_task();
extern eMOTORDIRECTION eWindowDirection;
extern eLATCHPOSITION eLatchPosition;
extern "C" TIM_HandleTypeDef htim3;
extern StaticQueue_t xStaticQueue;
extern QueueHandle_t xLatchQueue;
extern uint8_t ucQueueStorageArea[ QUEUE_LENGTH * ITEM_SIZE ];



#endif