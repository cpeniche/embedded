#pragma once 

#ifdef __cplusplus
extern "C"{
#endif

/* --------------------- SPI Definitions and static variables ------------------ */

#define PIN_NUM_CLK   GPIO_NUM_7
#define PIN_NUM_MISO  GPIO_NUM_8
#define PIN_NUM_MOSI  GPIO_NUM_9

/*********************** */
#define NUM_BITS 16
extern void ResetSleepTimer(void);

#ifdef __cplusplus
}
#endif