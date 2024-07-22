#ifndef _motors_h_
#define _motors_h_

#ifdef __cplusplus
extern "C"{
#endif

typedef enum{
  eLATCHMOTORDRIVER,
  eWINDOWMOTORDRIVER,
  eMIRRORMOTORDRIVER,
  eNOMOTOR
}eMOTOR_TYPE;


typedef struct
{
  union{
    uint16_t uSpiData;
    struct{
      uint16_t NA         : 1; //LSB  
      uint16_t uLabt      : 1;
      uint16_t uAddress   : 5;
      uint16_t uAccesType : 1;      
      uint16_t uData      : 8;
    };
  };
}__attribute__((packed))xTleTxMessageType;

typedef struct
{
  union{
    uint16_t uSpiData;
    struct{
      uint16_t uSpiErr : 1;  //lsb
      uint16_t uLE     : 1;     
      uint16_t uVSUV   : 1;
      uint16_t uVSOV   : 1;
      uint16_t uNPOR   : 1;
      uint16_t uTSD    : 1;
      uint16_t uTPW    : 1;
      uint16_t NA      : 1;
      uint16_t uData   : 8;
    };
  };
}__attribute__((packed))xTleRxMessageType;



#define HB1_LS_EN 0
#define HB1_HS_EN 1
#define HB2_LS_EN 2
#define HB2_HS_EN 3
#define HB3_LS_EN 4
#define HB3_HS_EN 5

#define MIRROR_DRIVER_MOVE_UP     (1<<HB3_LS_EN | 1<<HB1_HS_EN)
#define MIRROR_DRIVER_MOVE_DOWN   (1<<HB1_LS_EN | 1<<HB3_HS_EN)
#define MIRROR_DRIVER_MOVE_LEFT   (1<<HB3_LS_EN | 1<<HB2_HS_EN)
#define MIRROR_DRIVER_MOVE_RIGHT  (1<<HB2_LS_EN | 1<<HB3_HS_EN)

#define TLEMOTORENABLE      GPIO_NUM_2
#define TLEMOTORCHIPSELECT  GPIO_NUM_1

#define configMOTORQUEUESIZE  10
#define configITEMSIZE        sizeof(eMOTOR_TYPE)

#define LATCHMOTORPHASE  GPIO_NUM_5
#define LATCHMOTORENABLE GPIO_NUM_6

#define WINDOWMOTOR_INA   GPIO_NUM_14
#define WINDOWMOTOR_INB   GPIO_NUM_15
#define WINDOWMOTOR_PWM   GPIO_NUM_16
#define WINDOWMOTOR_ENA   GPIO_NUM_17
#define WINDOWMOTOR_ENB   GPIO_NUM_18

#define CONTROLREGISTER 0x0
#define GLOBALSTATUSREGISTER 0x6
#define OP1STATUSREGISTER 0x16
#define OP2STATUSREGISTER 0x1

extern void vMotorsTask(void *pvParameters);

extern eMOTOR_TYPE eMotorFunctionSelect;
extern QueueHandle_t xMotorQueue;

#ifdef __cplusplus
}
#endif

#endif