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
#include "buttons.h"
#include "dictionary.h"
#include "motors.h"
#include "spidrivermodel.h"
#include "drivers/include/espspi.h"
#include "main.h"

static const char *TAG = "Buttons_Task";
#define PARALLEL_LOAD GPIO_NUM_9

gpio_config_t io_conf = {
    .pin_bit_mask = 1 << PARALLEL_LOAD,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
};

uint16_t uPreviousDriverControllerButtons;

typedef struct
{
  stMessageHeader xHeader;
  uint16_t data;
  /* data */
} __attribute__((packed)) stMessageType;

stMessageType xMessage =
{
              /*mesageid , readwrite, datatype*/
    .xHeader= {0x55,1,eUNSIGNED16},
    .data={0}
};

spi_device_handle_t xprvSpiHandle;
uint8_t uTxBuffer[2] = {0};
uint8_t uRxBuffer[2] = {0};

/*********** vSpiTask* ***************/

void vButtonsTask(void *pvParameters)
{

  EventBits_t pxESPNowEvents;
  uint8_t uprvRetry = 0;
  
  gpio_config(&io_conf);
  gpio_set_level(PARALLEL_LOAD, 1);

  

  EspSpiBuilder *xSpiBuild = new EspSpiBuilder(SPI2_HOST, SPI_DMA_CH_AUTO);
  



  xprvEspSpiBusConfiguration.xBuild();

  
  // xSpiBuild->xBuildBusConfigure(xprvEspSpiBusConfiguration);
  // xSpiBuild->xBuildDevice(xprvEspSpiDevice);
  // xSpiBuild->xBuildTransaction(xprEspvSpiTransaction);

  //SpiDriver *xSpiDriver = static_cast<SpiDriver *>(xSpiBuild->xBuild());

  // memset(&prvxSpiTransaction, 0, sizeof(prvxSpiTransaction)); // Zero out the transaction
  // prvxSpiTransaction.length = 16;                             // Command is 8 bits
  // prvxSpiTransaction.tx_buffer = NULL;                        // The data is the cmd itself
  // prvxSpiTransaction.rx_buffer = &(xMessage.data);
  // prvxSpiTransaction.user = (void *)0; // D/C needs to be set to 0
  // prvxSpiTransaction.rxlength = 16;

  ESP_LOGI(TAG, "Buttons Task Started");
  while (1)
  {
    esp_err_t pxReturnCode;
    /* Latch parallel inputs*/
    gpio_set_level(PARALLEL_LOAD, 0);
    vTaskDelay(pdMS_TO_TICKS(5));
    gpio_set_level(PARALLEL_LOAD, 1);

    // xSpiDriver->Transmit();
    // pxReturnCode = *(static_cast<esp_err_t *>(xSpiDriver->GetError()));
    // uPreviousDriverControllerButtons = *(static_cast<uint16_t *>(xSpiDriver->GetReceiveData()));
    // pxReturnCode = spi_device_polling_transmit(spi, &prvxSpiTransaction);

    assert(pxReturnCode == ESP_OK);

    pxESPNowEvents = xEventGroupGetBits(xESPnowEventGroupHandle);

    if (uPreviousDriverControllerButtons != xMessage.data)
    {

      ResetSleepTimer();
      ESP_LOGI(TAG, "SPI Buttons Changed to : %x", xMessage.data);
      uPreviousDriverControllerButtons = xMessage.data;

      /* Send Data over ESP Now*/
      pxReturnCode = esp_now_send(uBroadCastMAC, (uint8_t *)&xMessage, sizeof(xMessage));
      ESP_LOGI(TAG, "esp_now_send function return code : %d", pxReturnCode);

      /* if send function return sucess*/
      if (ESP_NOW_SEND_SUCCESS == pxReturnCode)
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
            esp_now_send(uDriverControlMAC, (uint8_t *)&xMessage, sizeof(xMessage));
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

void vButtonsCallBack(uint8_t uprvParameters)
{

  eMOTOR_TYPE xprivItemToQueue;

  if ((DOOR_SIGNALS & uButtons) != 0x0)
  {
    xprivItemToQueue = eLATCHMOTORDRIVER;
    xQueueSendToBack(xMotorQueue, &xprivItemToQueue, 10);
  }

  if ((WINDOW_SIGNALS & uButtons) != 0x0)
  {
    xprivItemToQueue = eWINDOWMOTORDRIVER;
    xQueueSendToBack(xMotorQueue, &xprivItemToQueue, 10);
  }

  if ((MIRROR_SIGNALS & uButtons) != 0x0)
  {
    xprivItemToQueue = eMIRRORMOTORDRIVER;
    xQueueSendToBack(xMotorQueue, &xprivItemToQueue, 10);
  }

  gpio_set_level(WINDOWMOTOR_PWM, 0);
}