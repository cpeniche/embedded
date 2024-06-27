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

/* --------------------- SPI Definitions and static variables ------------------ */

#define LCD_HOST    HSPI_HOST
#define PARALLEL_LOAD 25
#define PIN_NUM_CLK   26
#define PIN_NUM_MISO  27
/*********************** */
#define NUM_BITS 16
static const char *TAG = "Spi_Task";


gpio_config_t io_conf = {
    .intr_type = GPIO_INTR_DISABLE,
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = 1<<PARALLEL_LOAD,
    .pull_down_en = 0,
    .pull_up_en = 0
};

uint8_t uDriverControllerButtons[2]={0};
uint8_t uPreviousDriverControllerButtons[2]={0};

spi_device_handle_t spi;

/* define spi bus configuration */
spi_bus_config_t buscfg=
{
  .miso_io_num=PIN_NUM_MISO,
  .mosi_io_num=-1,//PIN_NUM_MOSI,
  .sclk_io_num=PIN_NUM_CLK,
  .quadwp_io_num=-1,
  .quadhd_io_num=-1,
  .max_transfer_sz=NUM_BITS
};

/* define spi device configuration */
spi_device_interface_config_t devcfg=
{
  .clock_speed_hz=1*1000*1000,           //Clock out at 1 MHz
  .mode=2,                               //SPI mode 0
  .spics_io_num=-1,                     //CS pin
  .queue_size=1,                          //We want to be able to queue 7 transactions at a time
  .pre_cb=NULL,                           //Specify pre-transfer callback to handle D/C line
};

/*********** vSpiTask* ***************/

void vSpiTask(void *pvParameters)
{
  esp_err_t pxReturnCode;
  EventBits_t pxESPNowEvents;
  uint8_t uprvRetry=0;

  gpio_config(&io_conf);
  gpio_set_level(PARALLEL_LOAD, 1);

  //Initialize the SPI bus
  pxReturnCode=spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
  ESP_ERROR_CHECK(pxReturnCode);

  //Attach the spi device to the SPI bus
  pxReturnCode=spi_bus_add_device(LCD_HOST, &devcfg, &spi);
  ESP_ERROR_CHECK(pxReturnCode);

  spi_transaction_t prvxSpiTransaction;
  memset(&prvxSpiTransaction, 0, sizeof(prvxSpiTransaction));       //Zero out the transaction
  prvxSpiTransaction.length=16;                    //Command is 8 bits
  prvxSpiTransaction.tx_buffer=NULL;               //The data is the cmd itself
  prvxSpiTransaction.rx_buffer=&uDriverControllerButtons;
  prvxSpiTransaction.user=(void*)0;                //D/C needs to be set to 0
  prvxSpiTransaction.flags = SPI_TRANS_MODE_OCT;   // 8 bit mode

  ESP_LOGI(TAG, "SPI Task Started");
  while(1)
  {
    
    /* Latch parallel inputs*/
    gpio_set_level(PARALLEL_LOAD, 0);
    vTaskDelay(pdMS_TO_TICKS(5));
    gpio_set_level(PARALLEL_LOAD, 1);

    /* send spi command to read inputs*/
    pxReturnCode =  spi_device_polling_transmit(spi,&prvxSpiTransaction);
    assert(pxReturnCode==ESP_OK);

    pxESPNowEvents = xEventGroupGetBits(xESPnowEventGroupHandle);

    if(*((uint16_t *)uPreviousDriverControllerButtons) != *((uint16_t *)uDriverControllerButtons))
    {
      ESP_LOGI(TAG, "SPI Buttons Changed to : %x %x", uDriverControllerButtons[0], uDriverControllerButtons[1]);
      *((uint16_t *)uPreviousDriverControllerButtons) = *((uint16_t *)uDriverControllerButtons);

      /* Send Data over ESP Now*/  
      pxReturnCode = esp_now_send(uBroadCastMAC, uDriverControllerButtons, sizeof(uDriverControllerButtons));
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
            esp_now_send(uDriverControlMAC, uDriverControllerButtons, sizeof(uDriverControllerButtons));
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