#pragma once
#include <zephyr/drivers/pwm.h>

class nrfPwm : public pwmInterface {
public:
 
  nrfPwm(nrfPwm&) = delete;
  void operator=(const nrfPwm&) = delete;
  static nrfPwm *getInstance(void);
  int8_t setDutyCycle(uint8_t dutyCycle) override;
  int8_t setFrequency(float frequency) override;
  int8_t start() override;
  int8_t stop() override;
  void init(void);

private:
  struct pwm_dt_spec valve = PWM_DT_SPEC_GET(DT_NODELABEL(blue_pwm_led));

protected:
  nrfPwm(void){};
  static nrfPwm* _instance;  
};