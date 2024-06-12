
#ifndef _motor_h_
#define _motor_h_

#include <stdint.h>
#include "dictionary.h"
#include "halIncludes.h"

enum MOTOR_DIR
{
  STOP=0,
  UP,
  DOWN
};

extern void Motor_Callback(mode_type mode);
extern void Motor_task();
extern MOTOR_DIR window_dir;
extern "C" TIM_HandleTypeDef htim3;


#endif