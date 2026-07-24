#pragma once
#include "pwmInterface.h"
#include <zephyr/drivers/pwm.h>

class nrfPwm : public pwmInterface {
public:
 
  nrfPwm(nrfPwm&) = delete;
  void operator=(const nrfPwm&) = delete;
  static nrfPwm *getInstance(void);
  int8_t setDutyCycle(uint8_t dutyCycle) override;
  int8_t setFrequency(uint32_t period) override;
  int8_t start() override;
  int8_t stop() override;

private:
  struct pwm_dt_spec _valve = PWM_DT_SPEC_GET(DT_NODELABEL(valve_1));
  uint32_t _dutyCycle=50;

protected:
  nrfPwm(void);
  static nrfPwm* _instance;  
};