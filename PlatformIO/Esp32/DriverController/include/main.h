#pragma once 


/* --------------------- SPI Definitions and static variables ------------------ */

#define PIN_NUM_CLK   GPIO_NUM_7
#define PIN_NUM_MISO  GPIO_NUM_8
#define PIN_NUM_MOSI  GPIO_NUM_4

/*********************** */
#define NUM_BITS 16
extern spi_device_handle_t spi;
extern void ResetSleepTimer(void);