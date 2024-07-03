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
#include "motors.h"


#define LATCHMOTORPHASE     5
#define LATCHMOTORENABLE    6

uint8_t uprvSendLatchMotorPulse = 0;
static const char *TAG = "Motors_Task";


gpio_config_t xprvMotorsIOConfiguration = {
  .intr_type = GPIO_INTR_DISABLE,
  .mode = GPIO_MODE_OUTPUT,
  .pin_bit_mask = 1<<LATCHMOTORPHASE | 1<< LATCHMOTORENABLE,
  .pull_down_en = 1,
  .pull_up_en = 0
};

uint16_t uButtons;

void vMotorsTask(void *pvParameters)
{
 
  gpio_config(&xprvMotorsIOConfiguration);
  gpio_set_level(LATCHMOTORPHASE,  0);
  gpio_set_level(LATCHMOTORENABLE, 0);

  ESP_LOGI(TAG, "Motors Task Started  ");

  while(1)
  {
      if(uprvSendLatchMotorPulse)
      {
        gpio_set_level(LATCHMOTORENABLE,1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(LATCHMOTORENABLE,0);
        uprvSendLatchMotorPulse=0;
      }     
  }
}

void vMotorsCallBack(uint8_t uprvParameters)
{
  if(uprvParameters == WRITE)
  {
    if((uButtons & DOOR_CLOSE) == DOOR_CLOSE)
      gpio_set_level(LATCHMOTORPHASE,  0);    
    if((uButtons & DOOR_OPEN) == DOOR_OPEN)
      gpio_set_level(LATCHMOTORPHASE,  1);
    uprvSendLatchMotorPulse=1;
  }
}

