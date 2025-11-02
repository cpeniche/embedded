#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include "motorInterface.h"
#include "drv8838.h"
#define ACTIVEPULSEMS   50

#if !DT_NODE_EXISTS(DT_NODELABEL(drv8838_enable))
#error "Overlay for drv8838 enable pin not defined."
#endif
#if !DT_NODE_EXISTS(DT_NODELABEL(drv8838_phase))
#error "Overlay for drv8838 enable pin not defined."
#endif

static constexpr struct gpio_dt_spec enable =
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(drv8838_enable), gpios, {0}); 
static constexpr struct gpio_dt_spec phase =
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(drv8838_phase), gpios, {0}); 


drv8838::drv8838()
{

  if(gpio_is_ready_dt(&enable)) 
  {	
    gpio_pin_configure_dt(&enable,GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&phase,GPIO_OUTPUT_INACTIVE);
  }
}

void drv8838::Right(void)
{
}

void drv8838::Left(void)
{
}

void drv8838::Down(void)
{  
  gpio_pin_set_dt(&phase,0);
  gpio_pin_set_dt(&enable,1);
  k_sleep(K_MSEC(ACTIVEPULSEMS));
  Idle();
}

void drv8838::Up(void)
{  
  gpio_pin_set_dt(&phase,1);
  gpio_pin_set_dt(&enable,1);
  k_sleep(K_MSEC(ACTIVEPULSEMS));
  Idle();
}

void drv8838::Idle(void)
{
  gpio_pin_set_dt(&enable,0);
}
