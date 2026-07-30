#include "i2cInterface.hpp"
#include "aht20Interface.hpp"

#define INIT 0xBE

class aht20 : public aht20Interface
{

public:
  aht20(aht20 &) = delete;
  void operator=(const aht20 &) = delete;
  static aht20 *getInstance(void);
  int8_t Init() override;
  float ReadHumidity(void) override;
  float ReadTemperature(void) override;
  int8_t TriggerMeasurement(void (*delayFunc)(uint32_t), uint32_t ms) override;

protected:
  aht20();
  static aht20 *_instance;
 

private:
  i2cInterface *bus = nullptr;
  uint8_t _rxBuffer[7] = {0};
  uint32_t _humidity;
  uint32_t _temperature;
};
