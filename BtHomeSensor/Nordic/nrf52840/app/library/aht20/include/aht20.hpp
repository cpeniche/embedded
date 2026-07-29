#include "at20Interface.hpp"


class aht20: public aht20Interface{

public:
  aht20(aht20&) = delete;
  void operator=(const aht20&) = delete;
  static aht20 *getInstance(void);
  virtual int8_t Init() override;
  virtual int8_t ReadHumidity(void *) override;
  virtual int8_t ReadTemperature(void *) override;
  void setBusInterface();
 
private:

  
}
