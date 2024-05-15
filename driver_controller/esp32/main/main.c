#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"


#define LCD_HOST    HSPI_HOST
#define PIN_NUM_MISO 36
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  27
#define PIN_NUM_CS   25
#define PARALLEL_LINES 16


void app_main(void)
{

  uint8_t cmd=0xAA;

  esp_err_t ret;
  spi_device_handle_t spi;
  spi_bus_config_t buscfg={
      .miso_io_num=PIN_NUM_MISO,
      .mosi_io_num=PIN_NUM_MOSI,
      .sclk_io_num=PIN_NUM_CLK,
      .quadwp_io_num=-1,
      .quadhd_io_num=-1,
      .max_transfer_sz=PARALLEL_LINES
  };
  spi_device_interface_config_t devcfg={
  #ifdef CONFIG_LCD_OVERCLOCK
      .clock_speed_hz=26*1000*1000,           //Clock out at 26 MHz
  #else
      .clock_speed_hz=10*1000*1000,           //Clock out at 10 MHz
  #endif
      .mode=0,                                //SPI mode 0
      .spics_io_num=PIN_NUM_CS,               //CS pin
      .queue_size=7,                          //We want to be able to queue 7 transactions at a time
      .pre_cb=NULL,  //Specify pre-transfer callback to handle D/C line
  };
  //Initialize the SPI bus
  ret=spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
  ESP_ERROR_CHECK(ret);
  //Attach the LCD to the SPI bus
  ret=spi_bus_add_device(LCD_HOST, &devcfg, &spi);
  ESP_ERROR_CHECK(ret);


  spi_transaction_t t;
  memset(&t, 0, sizeof(t));       //Zero out the transaction
  t.length=8;                     //Command is 8 bits
  t.tx_buffer=&cmd;               //The data is the cmd itself
  t.user=(void*)0;                //D/C needs to be set to 0
  t.flags = SPI_TRANS_MODE_OCT;   //Keep CS active after data transfer

  while(1)
  {
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

}

