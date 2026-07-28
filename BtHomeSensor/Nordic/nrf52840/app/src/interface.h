#ifndef __INTERFACE_HPP__
#define __INTERFACE_HPP__

#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif

  int bmp280_i2c_init(uint8_t deviceAddress);
  int bmp280_i2c_deinit(uint8_t deviceAddress);
  int bmp280_read_array(uint8_t deviceAddress, uint8_t startRegisterAddress, uint8_t *data, uint8_t dataLength);
  int bmp280_write_array(uint8_t deviceAddress, uint8_t startRegisterAddress, uint8_t *data, uint8_t dataLength);
  int delay_function(uint32_t delayMS);
  int power_function(float x, float y, float *result);
  int getMode();

#ifdef __cplusplus
}
#endif

#endif
