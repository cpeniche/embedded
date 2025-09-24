#include "spiBuilder.h"
#include "tle9104.h"


void Tle94103::WriteRegister(uint8_t u8RegAddres, uint8_t *ptrU8Data)
{

  uint8_t u8Buffer[]={W_R(1) | (u8RegAddres << 2) | LABT(1) | 0x1, *ptrU8Data };
  spi->Write(u8Buffer,2);

}