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

static const char *TAG = "Motors_Task";

/* The variable used to hold the queue's data structure. */
static StaticQueue_t xMotorsStaticQueue;

/* The array to use as the queue's storage area. This must be at least
(uxQueueLength * uxItemSize) bytes. */
uint8_t ucMotorQueueStorageArea[ configMOTORQUEUESIZE * configITEMSIZE ];
QueueHandle_t xMotorQueue;


void vprvLatchMotorDriver();
void vprvWindowMotorDriver();
void vprvMirrorMotorDriver();

void (*vMotorsFunctions[3])(void)=
{
  vprvLatchMotorDriver,
  vprvWindowMotorDriver,
  vprvMirrorMotorDriver
};


gpio_config_t xprvMotorsIOConfiguration = {
  .intr_type = GPIO_INTR_DISABLE,
  .mode = GPIO_MODE_OUTPUT,
  .pin_bit_mask = 1<<LATCHMOTORPHASE | 1<< LATCHMOTORENABLE | 1<<WINDOWMOTOR_INA | 1<<WINDOWMOTOR_INB | \
                  1<<WINDOWMOTOR_ENA | 1<< WINDOWMOTOR_ENB | 1<< WINDOWMOTOR_PWM,
  .pull_down_en = 1,
  .pull_up_en = 0
};

uint16_t uButtons;
eMOTOR_TYPE eMotorFunctionSelect=eNOMOTOR;


void vMotorsTask(void *pvParameters)
{
 

  /* Create a queue capable of containing 10 uint64_t values. */
  xMotorQueue = xQueueCreateStatic( configMOTORQUEUESIZE,
                                configITEMSIZE,
                                ucMotorQueueStorageArea,
                                &xMotorsStaticQueue );

  /* ucMotorQueueStorageArea was not NULL so xMotorQueue should not be NULL. */
  configASSERT( xMotorQueue );                                

  gpio_config(&xprvMotorsIOConfiguration);
  gpio_set_level(LATCHMOTORPHASE,  0);
  gpio_set_level(LATCHMOTORENABLE, 0);

  ESP_LOGI(TAG, "Motors Task Started  ");

  while(1)
  {
      /*Wait for a Message from the Buttons Callback*/
      if(xQueueReceive(xMotorQueue,&eMotorFunctionSelect,portMAX_DELAY) == pdPASS)
      {
        if(NULL != vMotorsFunctions[eMotorFunctionSelect])
          vMotorsFunctions[eMotorFunctionSelect]();
      }
  }
}

void vprvLatchMotorDriver()
{

  ESP_LOGI(TAG, " %s Executed",__func__);
  if(uButtons & DOOR_CLOSE )
    gpio_set_level(LATCHMOTORPHASE,  0);
  else
    gpio_set_level(LATCHMOTORPHASE,  1);
  
  gpio_set_level(LATCHMOTORENABLE, 1);
  vTaskDelay(pdMS_TO_TICKS(100));
  gpio_set_level(LATCHMOTORENABLE, 0);

}

void vprvWindowMotorDriver()
{
  
  ESP_LOGI(TAG, " %s Executed",__func__);

  if(LEFT_WINDOW_UP & uButtons)
  {
    gpio_set_level(WINDOWMOTOR_INA,  0);
    gpio_set_level(WINDOWMOTOR_INB,  1);
  }
  if(LEFT_WINDOW_DOWN & uButtons)
  {
    gpio_set_level(WINDOWMOTOR_INA,  1);
    gpio_set_level(WINDOWMOTOR_INB,  0);
  }
  gpio_set_level(WINDOWMOTOR_PWM,  1);
}

void vprvMirrorMotorDriver()
{
  ESP_LOGI(TAG, " %s Executed",__func__);
}