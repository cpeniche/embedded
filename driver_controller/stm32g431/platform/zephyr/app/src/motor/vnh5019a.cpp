#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include "motorInterface.h"
#include "can.h"
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


vnh5019a::vnh5019a(can *interface)
{
  CanDriver = interface;
  gpio_pin_configure_dt(&ina,GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&ena,GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&inb,GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&enb,GPIO_OUTPUT_INACTIVE);
}

void vnh5019a::Right(uint8_t side)
{
}

void vnh5019a::Left(uint8_t side)
{
}

void vnh5019a::Down(uint8_t side)
{  
  canTxBufer[0]=0x0;
  canTxBufer[0]=0x1;
  if (side == 0x0)
  {
    gpio_pin_set_dt(&ina,1);
    gpio_pin_set_dt(&inb,0);
    gpio_pin_set_dt(&ena,1);
    gpio_pin_set_dt(&enb,1);
  }
  else 
    CanDriver->sendCanMsg(canTxBufer);
}

void vnh5019a::Up(uint8_t side)
{  
  canTxBufer[0]=0x0;
  canTxBufer[0]=0x2;
  if (side == 0x0)
  {
    gpio_pin_set_dt(&ina,0);
    gpio_pin_set_dt(&inb,1);
    gpio_pin_set_dt(&ena,1);
    gpio_pin_set_dt(&enb,1);
  }
  else 
    CanDriver->sendCanMsg(canTxBufer);
}

void vnh5019a::Idle(uint8_t side)
{
  gpio_pin_set_dt(&ena,0);
  gpio_pin_set_dt(&enb,0);
}