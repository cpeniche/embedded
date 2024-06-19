#include "dictionary.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "motor.h"



enum motor_state
{
  IDLE,
  SETTING_UP_MOVE,
  SETTING_DOWN_MOVE,
  RUNNING,
  STOP_MOVE
};

void idle();
void setting_up_move();
void setting_down_move();
void running();
void stop();

void (*func_state[5])(void)=
{
  idle,
  setting_up_move,
  setting_down_move,
  running,
  stop
};


eMOTORDIRECTION eWindowDirection=eSTOP;
eMOTORDIRECTION ePreviousWindowDirection=eSTOP;
StaticQueue_t xStaticQueue;
QueueHandle_t xLatchQueue;
eLATCHPOSITION eLatchPosition;
uint8_t ucQueueStorageArea[ QUEUE_LENGTH * ITEM_SIZE ];

void vMotorCallback(eReadWrite mode)
{
  
  /* Stop Pwm signal*/
  xPwmStop;

  /* if the direction is different from previous one
     change to new direction*/
  if(eWindowDirection != ePreviousWindowDirection)
  {
    ePreviousWindowDirection = eWindowDirection;
    if(eWindowDirection == eMOVEDOWN)
       xSetPinState(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
    else
      if(eWindowDirection == eMOVEUP)
        xSetPinState(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);
    xPwmStart;
  }
}

void vDoorLatchCallback(eReadWrite mode)
{
  xQueueSend(xLatchQueue,&eLatchPosition, 10);
}

#if 0
bool start_pwm=false;

void Motor_task()
{

  if(func_state[state] != nullptr)
    func_state[state]();    
}

void idle()
{
  if(window_dir == UP)
    state=SETTING_UP_MOVE;
  else if(window_dir == DOWN)
    state=SETTING_DOWN_MOVE;
}

void setting_up_move()
{
  //HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
  state = RUNNING;
  new_movement=false;
}

void setting_down_move()
{  
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);
  //HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
  state = RUNNING;
  new_movement=false;
  
}

void running()
{
  if(new_movement)
  {
    HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_1);
    if(window_dir == UP )
      state=SETTING_UP_MOVE;
    else if(window_dir == DOWN)
      state=SETTING_DOWN_MOVE;
    else
      state=STOP_MOVE;
  }
}

void stop()
{
  HAL_TIM_PWM_Stop(&htim3,1);
  //HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);
  //HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
  state=IDLE;
}
#endif