
#include <zephyr/sys/printk.h>
#include "pwmInterface.h"
#include "nrfPwm.h"

nrfPwm* nrfPwm::_instance=nullptr;

nrfPwm *nrfPwm::getInstance(void)
{
  if (_instance == nullptr)
  {
    static nrfPwm instance;
    _instance = new nrfPwm();
    _instance->init();
  }    
  return _instance;
}

int8_t nrfPwm::setDutyCycle(uint8_t dutyCycle)
{
  uint32_t pulse=valve.period/100*dutyCycle;
  return pwm_set_dt(&valve, valve.period,pulse);
}

int8_t nrfPwm::setFrequency(float frequency)
{
  return 0;
}

int8_t nrfPwm::start()
{
  return 0;
}

int8_t nrfPwm::stop()
{
  return 0;
}

void nrfPwm::init(void)
{

  uint32_t pulse=0;
  if(pwm_is_ready_dt(&valve))
  {
    pwm_set_dt(&valve, valve.period, 0);
  }
  else
  {
    printk("PWM device not ready\n");
  }
}
