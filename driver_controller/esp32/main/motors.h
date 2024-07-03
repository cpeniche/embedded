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


extern void vMotorsTask(void *pvParameters);

extern eMOTOR_TYPE eMotorFunctionSelect;
extern QueueHandle_t xMotorQueue;

#endif