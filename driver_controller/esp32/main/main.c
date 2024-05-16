#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"


#define LCD_HOST    HSPI_HOST
#define PIN_NUM_MISO 36
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  27
#define PARALLEL_LOAD   25
#define PARALLEL_LINES 16

static const char *TAG = "Spi_Task";

gpio_config_t io_conf = {};

uint8_t switch_buttons[2]={0};

spi_device_handle_t spi;

/* define spi bus configuration */
spi_bus_config_t buscfg=
{
  .miso_io_num=PIN_NUM_MISO,
  .mosi_io_num=PIN_NUM_MOSI,
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

  //Initialize the SPI bus
	ret=spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
	ESP_ERROR_CHECK(ret);

	//Attach the spi device to the SPI bus
	ret=spi_bus_add_device(LCD_HOST, &devcfg, &spi);
	ESP_ERROR_CHECK(ret);

	spi_transaction_t t;
  memset(&t, 0, sizeof(t));       //Zero out the transaction
  t.length=16;                    //Command is 8 bits
  t.tx_buffer=NULL;               //The data is the cmd itself
  t.rx_buffer=&switch_buttons;
  t.user=(void*)0;                //D/C needs to be set to 0
  t.flags = SPI_TRANS_MODE_OCT;   // 8 bit mode

	while(1)
	{
	  gpio_set_level(PARALLEL_LOAD, 0);
	  vTaskDelay(1 / portTICK_PERIOD_MS);
	  gpio_set_level(PARALLEL_LOAD, 1);
	  ret =  spi_device_polling_transmit(spi,&t);
	  assert(ret==ESP_OK);
	  vTaskDelay(500 / portTICK_PERIOD_MS);
	  ESP_LOGI(TAG,"bottons pressed =%x %x\n",switch_buttons[0], switch_buttons[1]);

	}
}

void app_main(void)
{

  //disable interrupt
  io_conf.intr_type = GPIO_INTR_DISABLE;
  //set as output mode
  io_conf.mode = GPIO_MODE_OUTPUT;
  //bit mask of the pins that you want to set,e.g.GPIO18/19
  io_conf.pin_bit_mask = 1<<PARALLEL_LOAD;
  //disable pull-down mode
  io_conf.pull_down_en = 0;
  //disable pull-up mode
  io_conf.pull_up_en = 0;
  //configure GPIO with the given settings
  gpio_config(&io_conf);

  gpio_set_level(PARALLEL_LOAD, 1);

  xTaskCreate(vSpiTask, "buttons_task", 2048, (void *)0, 4, NULL);

}




