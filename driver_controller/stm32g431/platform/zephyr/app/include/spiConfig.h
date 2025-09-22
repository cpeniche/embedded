#pragma once
#define SPI_NODE DT_NODELABEL(spi1)

extern struct device *spi1_dev;
extern uint8_t txMemoryBuffer[16];

int sendDrvMsg();