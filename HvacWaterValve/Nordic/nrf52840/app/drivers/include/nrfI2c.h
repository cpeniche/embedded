#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nrfI2cCppClass nrfI2cCppClass;

int8_t cppI2cRead(uint8_t *buffer, size_t len);
int8_t cppI2cWrite(uint8_t *buffer, size_t len);
int8_t cppIsI2cReady(void);
void   zephyrDelay(uint32_t delayMS);


#ifdef __cplusplus
}
#endif