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
  uint32_t ReadHumidity(void) override;
  uint32_t ReadTemperature(void) override;
  int8_t TriggerMeasurement(void (*delayFunc)(uint32_t), uint32_t ms) override;

protected:
  aht20();
  static aht20 *_instance;
  uint8_t data[7] = {0};

private:
  i2cInterface *bus = nullptr;
};
