
#include <stdint.h>
#include <stddef.h>
#include "i2cFactory.hpp"
#include "aht20.hpp"
#include "nrfI2c.hpp"

aht20 *aht20::_instance = nullptr;
struct i2c_dt_spec aht20Device = I2C_DT_SPEC_GET(DT_NODELABEL(aht20));
uint8_t triggerMeasurement[3] = {0xAC, 0x33, 0x00};

aht20 *aht20::getInstance(void)
{

  if (_instance == nullptr)
  {
    _instance = aht20::getInstance();
  }
  return _instance;
}

int8_t aht20::Init()
{
  return 0;
}

uint32_t aht20::ReadHumidity(void)
{

  return 0;
}

uint32_t aht20::ReadTemperature(void)
{

  return 0;
}

int8_t aht20::TriggerMeasurement(void (*delayFunc)(uint32_t), uint32_t ms)
{
  bus->SetDevice(&aht20Device);
  bus->Write(triggerMeasurement, sizeof(triggerMeasurement));
  delayFunc(ms);
  return bus->Read(data, 7);
}

aht20::aht20()
{
  uint8_t init = INIT;
  i2cFactory *factory = new i2cNrfI2cFactory();
  bus = factory->Factory();
  bus->SetDevice(&aht20Device);
  bus->Write(&init, 1);
}
