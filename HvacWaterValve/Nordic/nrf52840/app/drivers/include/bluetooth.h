#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

  int8_t initBluetooth(void);
  int updateBluetoothData(uint8_t *, uint8_t *, uint8_t *);

#ifdef __cplusplus
}
#endif
