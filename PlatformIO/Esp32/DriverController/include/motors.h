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

#define TLEMOTORENABLE      GPIO_NUM_10
#define TLEMOTORCHIPSELECT  GPIO_NUM_13

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

#define MIRROR_MOVE_UP 0x0c00
#define MIRROR_SELECT_LEFT 0x4000
#define MIRROR_SELECT_RIGHT 0x8000
#define MIRROR_MOVE_DOWN 0x3000
#define MIRROR_MOVE_LEFT 0x1400
#define MIRROR_MOVE_RIGHT 0x2800

#define LEFT_WINDOW_UP 0x0004
#define LEFT_WINDOW_DOWN 0x0008
#define RIGHT_WINDOW_ALL_DOWN 0x00c0
#define LEFT_WINDOW_ALL_DOWN 0x0018
#define RIGHT_WINDOW_UP 0x0020
#define RIGHT_WINDOW_DOWN 0x0040

#define DOOR_CLOSE 0x0002
#define DOOR_OPEN 0x0001

#define DOOR_SIGNALS (DOOR_CLOSE | DOOR_OPEN)
#define WINDOW_SIGNALS (LEFT_WINDOW_UP | LEFT_WINDOW_DOWN | LEFT_WINDOW_ALL_DOWN | \
                        RIGHT_WINDOW_UP | RIGHT_WINDOW_DOWN | RIGHT_WINDOW_ALL_DOWN)

#define MIRROR_SIGNALS (MIRROR_MOVE_UP | MIRROR_SELECT_LEFT | MIRROR_SELECT_RIGHT | MIRROR_MOVE_DOWN | MIRROR_MOVE_LEFT | MIRROR_MOVE_RIGHT)

extern void vMotorsTask(void *pvParameters);
extern void vESPNowMessageProcessButtonsCallBack(uint8_t, eDataTYPE, void *);

#ifdef __cplusplus
}
#endif

#endif