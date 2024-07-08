#ifndef _motors_h_
#define _motors_h_

typedef enum{
  eLATCHMOTORDRIVER,
  eWINDOWMOTORDRIVER,
  eMIRRORMOTORDRIVER,
  eNOMOTOR
}eMOTOR_TYPE;


#define configMOTORQUEUESIZE  10
#define configITEMSIZE        sizeof(eMOTOR_TYPE)

#define LATCHMOTORPHASE   5
#define LATCHMOTORENABLE  6

#define WINDOWMOTOR_INA   14
#define WINDOWMOTOR_INB   15
#define WINDOWMOTOR_PWM   16
#define WINDOWMOTOR_ENA   17
#define WINDOWMOTOR_ENB   18

extern void vMotorsTask(void *pvParameters);

extern eMOTOR_TYPE eMotorFunctionSelect;
extern QueueHandle_t xMotorQueue;

#endif