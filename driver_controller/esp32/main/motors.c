#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_now.h"
#include "espNow.h"
#include "buttons.h"
#include "dictionary.h"


#define LATCHOPEN     20
#define LATCHCLOSE    28


gpio_config_t xprvMotorsIOConfiguration = {
  .intr_type = GPIO_INTR_DISABLE,
  .mode = GPIO_MODE_OUTPUT,
  .pin_bit_mask = 1<<LATCHOPEN | 1<< LATCHCLOSE,
  .pull_down_en = 1,
  .pull_up_en = 0
};

uint16_t uButtons;

void vMotorsTask(void *pvParameters)
{
 
  gpio_config(&xprvMotorsIOConfiguration);
  gpio_set_level(LATCHOPEN,  0);
  gpio_set_level(LATCHCLOSE, 0);

  
}

void vMotorsCallBack(uint8_t uprvParameters)
{


}

