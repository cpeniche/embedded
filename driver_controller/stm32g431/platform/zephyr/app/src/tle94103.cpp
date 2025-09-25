#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(tle94103, LOG_LEVEL_DBG);
#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include "spiBuilder.h"
#include "tle94103.h"


void Tle94103::WriteRegister(uint8_t u8RegAddress, uint8_t *ptrU8Data)
{

  uint8_t u8Buffer[]={uint8_t(W_R(1) | 
                      ADDRESS(u8RegAddress) | 
                      LABT(1) | 0x1),
                      *ptrU8Data};
  spi->Write(u8Buffer,2);

}