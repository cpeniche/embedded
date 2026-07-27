
#include <zephyr/sys/printk.h>
#include "nrfPwm.h"

nrfPwm *nrfPwm::_instance = nullptr;

nrfPwm *nrfPwm::getInstance(void)
{
  if (_instance == nullptr)
  {
    _instance = new nrfPwm();
  }
  return _instance;
}

int8_t nrfPwm::setDutyCycle(uint8_t dutyCycle)
{
  uint32_t pulse = _valve.period / 100 * dutyCycle;
  _dutyCycle = dutyCycle;
  return pwm_set_dt(&_valve, _valve.period, pulse);
}

int8_t nrfPwm::setFrequency(uint32_t period)
{

  uint32_t pulse = period / 100 * _dutyCycle;
  return pwm_set_dt(&_valve, period, pulse);
}

int8_t nrfPwm::start()
{
  return 0;
}

int8_t nrfPwm::stop()
{
  return 0;
}

nrfPwm::nrfPwm(void)
{

  if (pwm_is_ready_dt(&_valve))
  {
    pwm_set_dt(&_valve, _valve.period, 0);
  }
  else
  {
    printk("PWM device not ready\n");
  }
}

extern "C" void DrivePwm(uint8_t index, uint8_t value)
{

  nrfPwm *pwm = nrfPwm::getInstance();

  printk("index = %u, value=%u\n", index, value);

  if (index == 0)
    pwm->setDutyCycle(value);
}
