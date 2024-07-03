#ifndef _motors_h_
#define _motors_h_


#define MIRROR_SELECT_LEFT  0x4000
#define MIRROR_SELECT_RIGHT 0x8000
#define MIRROR_MOVE_UP      0x0c00
#define MIRROR_MOVE_DOWN    0x3000
#define MIRROR_MOVE_LEFT    0x1400
#define MIRROR_MOVE_RIGHT   0x2800


#define LEFT_WINDOW_UP       0x0004
#define LEFT_WINDOW_DOWN     0x0008
#define LEFT_WINDOW_ALL_DOWN 0x0018
#define RIGHT_WINDOW_UP      0x0020
#define RIGHT_WINDOW_DOWN    0x0040
#define RIGHT_WINDOW_ALL_DOWN   0x00c0

#define DOOR_CLOSE      0x0002
#define DOOR_OPEN       0x0001


extern void vMotorsTask(void *pvParameters);
extern void vMotorsCallBack(uint8_t);
extern uint16_t uButtons;


#endif