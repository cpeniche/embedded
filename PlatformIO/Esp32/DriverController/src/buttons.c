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
#include "main.h"

static const char *TAG = "Buttons_Task";
#define PARALLEL_LOAD 9


gpio_config_t io_conf = {
    .intr_type = GPIO_INTR_DISABLE,
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = 1<<PARALLEL_LOAD,
    .pull_down_en = 0,
    .pull_up_en = 0
};

uint16_t uPreviousDriverControllerButtons;

typedef struct
{
  stMessageHeader xHeader;
  uint16_t data;
  /* data */
}__attribute__((packed)) stMessageType;


stMessageType xMessage=
{
  .xHeader.uMessageId=0x55,
  .xHeader.uReadWrite=1,
  .xHeader.eDataType=eUNSIGNED16
};

/*********** vSpiTask* ***************/

void vButtonsTask(void *pvParameters)
{
  
  EventBits_t pxESPNowEvents;
  uint8_t uprvRetry=0;
  spi_transaction_t prvxSpiTransaction;
  

  gpio_config(&io_conf);
  gpio_set_level(PARALLEL_LOAD, 1);

  
  memset(&prvxSpiTransaction, 0, sizeof(prvxSpiTransaction));       //Zero out the transaction
  prvxSpiTransaction.length=16;                    //Command is 8 bits
  prvxSpiTransaction.tx_buffer=NULL;               //The data is the cmd itself
  prvxSpiTransaction.rx_buffer=&(xMessage.data);
  prvxSpiTransaction.user=(void*)0;                //D/C needs to be set to 0
  prvxSpiTransaction.rxlength=16;


  ESP_LOGI(TAG, "Buttons Task Started");
  while(1)
  {
    esp_err_t pxReturnCode;
    /* Latch parallel inputs*/
    gpio_set_level(PARALLEL_LOAD, 0);
    vTaskDelay(pdMS_TO_TICKS(5));
    gpio_set_level(PARALLEL_LOAD, 1);

    /* send spi command to read inputs*/
    // if((xSpiSemaphoreStatus=xSemaphoreTake(xSpiSemaphoreHandle,portMAX_DELAY))==pdTRUE)
    // {
      pxReturnCode =  spi_device_polling_transmit(spi,&prvxSpiTransaction);
      //xSemaphoreGive(xSpiSemaphoreHandle);
   // }
    assert(pxReturnCode==ESP_OK);
    //assert(xSpiSemaphoreStatus==pdPASS);

    pxESPNowEvents = xEventGroupGetBits(xESPnowEventGroupHandle);

    if(uPreviousDriverControllerButtons != xMessage.data)
    {
      ESP_LOGI(TAG, "SPI Buttons Changed to : %x", xMessage.data);
      uPreviousDriverControllerButtons = xMessage.data;

      /* Send Data over ESP Now*/  
      pxReturnCode = esp_now_send(uBroadCastMAC, (uint8_t *)&xMessage, sizeof(xMessage));
      ESP_LOGI(TAG, "esp_now_send function return code : %d", pxReturnCode);    
      
      
      /* if send function return sucess*/
      if(ESP_NOW_SEND_SUCCESS == pxReturnCode)
      {
        
        /* Check with callback function for the 
           receiver to acknowledge the message
        */
        while(pxESPNowEvents != (1<<eESPNOW_SEND_CB | 1<<eESPNOW_INIT_DONE))
        {
          pxESPNowEvents = xEventGroupGetBits(xESPnowEventGroupHandle);

          /* if we received an error, we retry configESPNOW_RETRANSMIT_MAX_RETRIES times*/
          if(pxESPNowEvents == (1<<eESPNOW_TRANSMIT_ERROR | 1<<eESPNOW_INIT_DONE))
          {
            ESP_LOGI(TAG, "ESP_NOW Transmit Error, Retry");
            esp_now_send(uDriverControlMAC, (uint8_t *)&xMessage, sizeof(xMessage));
            xEventGroupClearBits(xESPnowEventGroupHandle, 1<<eESPNOW_TRANSMIT_ERROR); 
            uprvRetry++;
            vTaskDelay(pdMS_TO_TICKS(2000));
          }
          
          /* Max retries reached, message lost */
          if(configESPNOW_RETRANSMIT_MAX_RETRIES == uprvRetry) 
          {
            uprvRetry=0;
            ESP_LOGI(TAG, "SPI Message Not Send"); 
            xEventGroupClearBits(xESPnowEventGroupHandle, 1<<eESPNOW_TRANSMIT_ERROR);
            break;
          }
        }                                         
        xEventGroupClearBits(xESPnowEventGroupHandle, 1<<eESPNOW_SEND_CB);            
      } 
    }    
  }
}

void vButtonsCallBack(uint8_t uprvParameters)
{
  
  eMOTOR_TYPE xprivItemToQueue;

  if((DOOR_SIGNALS & uButtons) != 0x0)
  {
    xprivItemToQueue=eLATCHMOTORDRIVER;
    xQueueSendToBack(xMotorQueue,&xprivItemToQueue,10);
  }

  if((WINDOW_SIGNALS & uButtons) != 0x0)
  {
    xprivItemToQueue=eWINDOWMOTORDRIVER;
    xQueueSendToBack(xMotorQueue,&xprivItemToQueue,10);
  }

  if((MIRROR_SIGNALS & uButtons) != 0x0)
  {
    xprivItemToQueue=eMIRRORMOTORDRIVER;
    xQueueSendToBack(xMotorQueue,&xprivItemToQueue,10);
  }

  gpio_set_level(WINDOWMOTOR_PWM,  0);
}