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
#define PARALLEL_LINES 16
//static const char *TAG = "Spi_Task";


gpio_config_t io_conf = {
    .intr_type = GPIO_INTR_DISABLE,
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = 1<<PARALLEL_LOAD,
    .pull_down_en = 0,
    .pull_up_en = 0
};

uint8_t uDriverControllerButtons[2]={0};

spi_device_handle_t spi;

/* define spi bus configuration */
spi_bus_config_t buscfg=
{
  .miso_io_num=PIN_NUM_MISO,
  .mosi_io_num=-1,//PIN_NUM_MOSI,
  .sclk_io_num=PIN_NUM_CLK,
  .quadwp_io_num=-1,
  .quadhd_io_num=-1,
  .max_transfer_sz=PARALLEL_LINES
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

void vSpiTask(void *pvParameters)
{
  esp_err_t ret;
  EventBits_t pxESPNowEvents;
  tx_task_action_t tx_action=TX_SEND_BUTTONS;

  gpio_config(&io_conf);
  gpio_set_level(PARALLEL_LOAD, 1);

  //Initialize the SPI bus
  ret=spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
  ESP_ERROR_CHECK(ret);

  //Attach the spi device to the SPI bus
  ret=spi_bus_add_device(LCD_HOST, &devcfg, &spi);
  ESP_ERROR_CHECK(ret);

  spi_transaction_t prvxSpiTransaction;
  memset(&prvxSpiTransaction, 0, sizeof(prvxSpiTransaction));       //Zero out the transaction
  prvxSpiTransaction.length=16;                    //Command is 8 bits
  prvxSpiTransaction.tx_buffer=NULL;               //The data is the cmd itself
  prvxSpiTransaction.rx_buffer=&uDriverControllerButtons;
  prvxSpiTransaction.user=(void*)0;                //D/C needs to be set to 0
  prvxSpiTransaction.flags = SPI_TRANS_MODE_OCT;   // 8 bit mode


  while(1)
  {
    gpio_set_level(PARALLEL_LOAD, 0);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    gpio_set_level(PARALLEL_LOAD, 1);
    ret =  spi_device_polling_transmit(spi,&prvxSpiTransaction);
    assert(ret==ESP_OK);
    pxESPNowEvents = xEventGroupGetBits(xESPnowEventGroupHandle);

    if(pxESPNowEvents == (1<<eESPNOW_SEND_CB | 1<<eESPNOW_INIT_DONE))
    {
        esp_now_send(uDriverControlMAC, &uDriverControllerButtons, sizeof(uDriverControllerButtons));
        xEventGroupClearBits(xESPnowEventGroupHandle, 1<<eESPNOW_SEND_CB);    
    }
    
  }
}