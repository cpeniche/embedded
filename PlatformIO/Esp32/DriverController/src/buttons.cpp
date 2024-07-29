#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
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
#include "spidrivermodel.h"
#include "drivers/include/espspi.h"
#include "dictionary.h"
#include "buttons.h"
#include "motors.h"

#include "main.h"


static const char *TAG = "Buttons_Task";
#define PARALLEL_LOAD GPIO_NUM_9

Buttons::Buttons()
{
  gpio_config_t io_conf = {
      .pin_bit_mask = 1 << PARALLEL_LOAD,
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  
  gpio_config(&io_conf);
  gpio_set_level(PARALLEL_LOAD, 1);

  /* Create Spi Configuration */
  SpiBuilder *xprvSpiBuilder = new EspSpiBuilder(nullptr, reinterpret_cast<uint8_t *>(&(xMessage.data)));
  Spi *xprvSpi = xprvSpiBuilder->xBuild();
  delete xprvSpiBuilder;

  xprvSpi->Init();
  xprvError = *(static_cast<esp_err_t *>(xprvSpi->GetError()));
  if (xprvError != ESP_OK)
  {
    ESP_LOGE(TAG, "Spi Initialization Error : %d", xprvError);
    assert(xprvError == ESP_OK);
  }
}

void Buttons::vReadButtons()
{
  /* Latch parallel inputs*/
  gpio_set_level(PARALLEL_LOAD, 0);
  vTaskDelay(pdMS_TO_TICKS(5));
  gpio_set_level(PARALLEL_LOAD, 1);
  xprvSpi->Transmit();
  xprvError = *(static_cast<esp_err_t *>(xprvSpi->GetError()));
  assert(xprvError == ESP_OK);
}

bool Buttons::bBottonsChanged()
{
  bool bprvReturn;
  bprvReturn = uprvPreviousButtonsState != xMessage.data;
  uprvPreviousButtonsState = xMessage.data;
  return bprvReturn;
}

void Buttons::vSendEspNowMessage(uint8_t *upprvReceiver)
{
  /* Send Data over ESP Now*/
  xprvError = esp_now_send(upprvReceiver, (uint8_t *)&xMessage, sizeof(xMessage));
}

bool Buttons::bIsThereAnError()
{
  return ESP_NOW_SEND_SUCCESS != xprvError;
}

esp_err_t Buttons::xGetError()
{

  ESP_LOGI(TAG, "esp_now_send function return code : %d", xprvError);
  return xprvError;
}



/*********** vSpiTask* ***************/

void vButtonsTask(void *pvParameters)
{

  EventBits_t pxESPNowEvents;
  uint8_t uprvRetry = 0;
  Buttons *buttons = new Buttons();

  ESP_LOGI(TAG, "Buttons Task Started");
  while (true)
  {
    buttons->vReadButtons();
    pxESPNowEvents = xEventGroupGetBits(xESPnowEventGroupHandle);

    /* Buttons Changed, need to send a new EspNow Message */
    if (buttons->bBottonsChanged())
    {

      ResetSleepTimer();

      buttons->vSendEspNowMessage(uBroadCastMAC);

      /* if send function return sucess*/
      if (buttons->bIsThereAnError())
      {

        /* Check with callback function for the
           receiver to acknowledge the message
        */
        while (pxESPNowEvents != (1 << eESPNOW_SEND_CB | 1 << eESPNOW_INIT_DONE))
        {
          pxESPNowEvents = xEventGroupGetBits(xESPnowEventGroupHandle);

          /* if we received an error, we retry configESPNOW_RETRANSMIT_MAX_RETRIES times*/
          if (pxESPNowEvents == (1 << eESPNOW_TRANSMIT_ERROR | 1 << eESPNOW_INIT_DONE))
          {
            ESP_LOGI(TAG, "ESP_NOW Transmit Error, Retry");
            buttons->vSendEspNowMessage(uBroadCastMAC);
            xEventGroupClearBits(xESPnowEventGroupHandle, 1 << eESPNOW_TRANSMIT_ERROR);
            uprvRetry++;
            vTaskDelay(pdMS_TO_TICKS(2000));
          }

          /* Max retries reached, message lost */
          if (configESPNOW_RETRANSMIT_MAX_RETRIES == uprvRetry)
          {
            uprvRetry = 0;
            ESP_LOGI(TAG, "SPI Message Not Send");
            xEventGroupClearBits(xESPnowEventGroupHandle, 1 << eESPNOW_TRANSMIT_ERROR);
            break;
          }
        }
        xEventGroupClearBits(xESPnowEventGroupHandle, 1 << eESPNOW_SEND_CB);
      }
    }
  }
}