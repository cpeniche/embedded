#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include "motorInterface.h"
#include "vnh5019a.h"

#if !DT_NODE_EXISTS(DT_NODELABEL(vnh5019a_ina))
#error "Overlay for drv8838 enable pin not defined."
#endif
#if !DT_NODE_EXISTS(DT_NODELABEL(vnh5019a_ena))
#error "Overlay for drv8838 enable pin not defined."
#endif
#if !DT_NODE_EXISTS(DT_NODELABEL(vnh5019a_inb))
#error "Overlay for drv8838 enable pin not defined."
#endif
#if !DT_NODE_EXISTS(DT_NODELABEL(vnh5019a_enb))
#error "Overlay for drv8838 enable pin not defined."
#endif

static constexpr struct gpio_dt_spec ina =
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(vnh5019a_ina), gpios, {0}); 
static constexpr struct gpio_dt_spec ena =
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(vnh5019a_ena), gpios, {0});
static constexpr struct gpio_dt_spec inb =
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(vnh5019a_inb), gpios, {0}); 
static constexpr struct gpio_dt_spec enb =
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(vnh5019a_enb), gpios, {0}); 


vnh5019a::vnh5019a()
{
  gpio_pin_configure_dt(&ina,GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&ena,GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&inb,GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&enb,GPIO_OUTPUT_INACTIVE);
}

void vnh5019a::Right(void)
{
}

void vnh5019a::Left(void)
{
}

void vnh5019a::Down(void)
{  
  gpio_pin_set_dt(&ina,1);
  gpio_pin_set_dt(&inb,0);
  gpio_pin_set_dt(&ena,1);
  gpio_pin_set_dt(&enb,1);
}

void vnh5019a::Up(void)
{  
  gpio_pin_set_dt(&ina,0);
  gpio_pin_set_dt(&inb,1);
  gpio_pin_set_dt(&ena,1);
  gpio_pin_set_dt(&enb,1);
}

void vnh5019a::Idle(void)
{
  gpio_pin_set_dt(&ena,0);
  gpio_pin_set_dt(&enb,0);
}