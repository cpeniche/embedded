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

static constexpr struct gpio_dt_spec pdrst =
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(ad7171_pdrst), gpios, {1});

#if 0
#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
    !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif
#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
  ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
                         DT_SPEC_AND_COMMA)};
#endif

adc::adc()
{

  if (gpio_is_ready_dt(&pdrst))
  {
    gpio_pin_configure_dt(&pdrst, GPIO_OUTPUT_ACTIVE);
  }

#if 0 
  /* Configure channels individually prior to sampling. */
  for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++)
  {
    if (!adc_is_ready_dt(&adc_channels[i]))
    {
      printk("ADC controller device %s not ready\n", adc_channels[i].dev->name);
    }
    else
    {
      err = adc_channel_setup_dt(&adc_channels[i]);
      if (err < 0)
      {
        printk("Could not setup channel #%d (%d)\n", i, err);
      }
    }
  }
#endif
}

int8_t adc::readInput(uint8_t *data, size_t size)
{

  gpio_pin_set_dt(&pdrst, 0);
  k_usleep(1);
  gpio_pin_set_dt(&pdrst, 1);
  k_sleep(K_MSEC(26));
  this->spi->Read(data, size);

#if 0

  for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++)
  {
    int32_t val_mv;

    printk("- %s, channel %d: ",
           adc_channels[i].dev->name,
           adc_channels[i].channel_id);

    (void)adc_sequence_init_dt(&adc_channels[i], &sequence);

    err = adc_read_dt(&adc_channels[i], &sequence);
    if (err < 0)
    {
      printk("Could not read (%d)\n", err);
      continue;
    }

    /*
     * If using differential mode, the 16 bit value
     * in the ADC sample buffer should be a signed 2's
     * complement value.
     */
    if (adc_channels[i].channel_cfg.differential)
    {
      val_mv = (int32_t)((int16_t)buf);
    }
    else
    {
      val_mv = (int32_t)buf;
    }
    printk("%" PRId32, val_mv);
    err = adc_raw_to_millivolts_dt(&adc_channels[i],
                                   &val_mv);
    /* conversion to mV may not be supported, skip if not */
    if (err < 0)
    {
      printk(" (value in mV not available)\n");
    }
    else
    {
      printk(" = %" PRId32 " mV\n", val_mv);
    }

    //printk("Internal reference = %d\n", adc_ref_internal(adc_channels[0].dev));
  }

#endif
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
