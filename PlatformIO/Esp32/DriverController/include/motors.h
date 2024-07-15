#ifndef _motors_h_
#define _motors_h_

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

#define TLEMOTORENABLE      2
#define TLEMOTORCHIPSELECT  1

#define configMOTORQUEUESIZE  10
#define configITEMSIZE        sizeof(eMOTOR_TYPE)

#define LATCHMOTORPHASE   5
#define LATCHMOTORENABLE  6

#define WINDOWMOTOR_INA   14
#define WINDOWMOTOR_INB   15
#define WINDOWMOTOR_PWM   16
#define WINDOWMOTOR_ENA   17
#define WINDOWMOTOR_ENB   18

#define CONTROLREGISTER 0x0
#define GLOBALSTATUSREGISTER 0x6
#define OP1STATUSREGISTER 0x16
#define OP2STATUSREGISTER 0x1

extern void vMotorsTask(void *pvParameters);

extern eMOTOR_TYPE eMotorFunctionSelect;
extern QueueHandle_t xMotorQueue;

#endif