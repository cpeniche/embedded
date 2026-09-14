
#include <stdint.h>
#include <stddef.h>
#include "i2cFactory.hpp"
#include "aht20.hpp"
#include "nrfI2c.hpp"

#define BSWAP_32(x) ((uint32_t)((((x) >> 24) & 0xff) |  \
                                (((x) >> 8) & 0xff00) | \
                                (((x) & 0xff00) << 8) | \
                                (((x) & 0xff) << 24)))

aht20 *aht20::_instance = nullptr;
struct i2c_dt_spec aht20Device = I2C_DT_SPEC_GET(DT_NODELABEL(aht20));
uint8_t triggerMeasurement[3] = {0xAC, 0x33, 0x00};

aht20 *aht20::getInstance(void)
{

  if (_instance == nullptr)
  {
    _instance = new aht20();
  }
  return _instance;
}

int8_t aht20::Init()
{
  uint8_t init = INIT;
  bus->SetDevice(&aht20Device);
  return bus->Write(&init, 1);
}

float aht20::ReadHumidity(void)
{
  return ((float)_humidity / 1048576.0f) * 100.0f;
}

float aht20::ReadTemperature(void)
{

  return ((float)_temperature / 1048576.0f) * 200.0f - 50.0f;
}

int8_t aht20::TriggerMeasurement(void (*delayFunc)(uint32_t), uint32_t ms)
{
  int8_t err=0;
  
  bus->SetDevice(&aht20Device);
  if((err = bus->Write(triggerMeasurement, sizeof(triggerMeasurement))) != 0)
    return err;
  delayFunc(ms);

  if((err = bus->Read(_rxBuffer, 7)) != 0)
    return err;

  _humidity = BSWAP_32((*((uint32_t *)&_rxBuffer[1]))) >> 12;
  _temperature = (BSWAP_32((*((uint32_t *)&_rxBuffer[3]))) >> 8) & 0x0FFFFF;
  return err;
}

aht20::aht20()
{
  i2cFactory *factory = new i2cNrfI2cFactory();
  bus = factory->Factory();  
}
