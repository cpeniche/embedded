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


#define LATCHOPEN     20
#define LATCHCLOSE    28


gpio_config_t xprvMotorsIOConfiguration = {
  .intr_type = GPIO_INTR_DISABLE,
  .mode = GPIO_MODE_OUTPUT,
  .pin_bit_mask = 1<<LATCHOPEN | 1<< LATCHCLOSE,
  .pull_down_en = 1,
  .pull_up_en = 0
};

uint8_t uprvMotorsPreviousState[2]={0,0};

void vMotorsTask(void *pvParameters)
{


  espnow_event_t xprvESPNowEvent;

  gpio_config(&xprvMotorsIOConfiguration);
  gpio_set_level(LATCHOPEN,  0);
  gpio_set_level(LATCHCLOSE, 0);

  while(1)
  {
    
    /* Message received from switch module */
    while (xQueueReceive(xESPNowQueue, &xprvESPNowEvent, portMAX_DELAY) == pdTRUE)
    {
      
      //xprvESPNowEvent.info.recv_cb.data

    }
  }
}