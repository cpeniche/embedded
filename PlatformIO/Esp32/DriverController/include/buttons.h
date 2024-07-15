#ifndef _buttons_h_
#define _buttons_h_


#define MIRROR_MOVE_UP      0x0c00
#define MIRROR_SELECT_LEFT  0x4000
#define MIRROR_SELECT_RIGHT 0x8000
#define MIRROR_MOVE_DOWN    0x3000
#define MIRROR_MOVE_LEFT    0x1400
#define MIRROR_MOVE_RIGHT   0x2800


#define LEFT_WINDOW_UP          0x0004
#define LEFT_WINDOW_DOWN        0x0008
#define RIGHT_WINDOW_ALL_DOWN   0x00c0
#define LEFT_WINDOW_ALL_DOWN    0x0018
#define RIGHT_WINDOW_UP         0x0020
#define RIGHT_WINDOW_DOWN       0x0040


#define DOOR_CLOSE      0x0002
#define DOOR_OPEN       0x0001

#define DOOR_SIGNALS   (DOOR_CLOSE | DOOR_OPEN)
#define WINDOW_SIGNALS (LEFT_WINDOW_UP | LEFT_WINDOW_DOWN | LEFT_WINDOW_ALL_DOWN | \
                        RIGHT_WINDOW_UP | RIGHT_WINDOW_DOWN | RIGHT_WINDOW_ALL_DOWN)

#define MIRROR_SIGNALS  (MIRROR_MOVE_UP | MIRROR_SELECT_LEFT | MIRROR_SELECT_RIGHT \
                         | MIRROR_MOVE_DOWN | MIRROR_MOVE_LEFT | MIRROR_MOVE_RIGHT)

#define BUTTON_TASK_STACK_SIZE  4096
extern void vButtonsTask(void *pvParameters);
extern uint16_t uButtons;
extern SemaphoreHandle_t xSpiSemaphoreHandle;

extern void vButtonsCallBack(uint8_t uprvParameters);


#endif