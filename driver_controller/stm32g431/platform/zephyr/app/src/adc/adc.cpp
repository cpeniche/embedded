#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
// #include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/spi.h>
#include "spiBuilder.h"
#include "inputInterface.h"
#include "adc.h"

#if !DT_NODE_EXISTS(DT_NODELABEL(ad7171_pdrst))
#error "Overlay for ad7171_pdrst pin not defined."
#endif

#if !DT_NODE_EXISTS(DT_NODELABEL(ad7171_pdrst_2))
#error "Overlay for ad7171_pdrst_2 pin not defined."
#endif

static /*constexpr*/ struct gpio_dt_spec pdrst =
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(ad7171_pdrst), gpios, {0});

static /*constexpr*/ struct gpio_dt_spec pdrst_2 =
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(ad7171_pdrst_2), gpios, {0});   
    
struct gpio_dt_spec *ptrPdrst[]={&pdrst, &pdrst_2};

adc::adc()
{

  if (gpio_is_ready_dt(&pdrst))
  {
    gpio_pin_configure_dt(&pdrst, GPIO_OUTPUT_ACTIVE);
  }

  if (gpio_is_ready_dt(&pdrst_2))
  {
    gpio_pin_configure_dt(&pdrst_2, GPIO_OUTPUT_ACTIVE);
  }
}

int8_t adc::readInput(uint8_t *data, size_t size)
{

  static uint8_t indx=0;

  gpio_pin_set_dt(ptrPdrst[indx], 1);
  
  k_sleep(K_MSEC(26));
  this->spi->Read(data, size);
  gpio_pin_set_dt(ptrPdrst[indx], 0);

  if (indx)
    indx=0;
  else
    indx++;

  return err;
}
uint8_t *adc::getInput(void)
{
  return nullptr;
}
bool adc::getDataReady(void)
{
  return false;
}