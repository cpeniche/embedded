#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/twai.h"
#include "soc/twai_periph.h"    // For GPIO matrix signal index


/******************** SPI CONFIGURATION VARIABLES *************************/

#define LCD_HOST    HSPI_HOST
#define PARALLEL_LOAD 12
#define PIN_NUM_CLK  13
#define PIN_NUM_MISO 14

#define PARALLEL_LINES 16
static const char *TAG = "Spi_Task";

gpio_config_t io_conf = {};

uint8_t switch_buttons[2]={0};

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


/********************* CAN Configuration variables  *********************/

#define PING_PERIOD_MS          250
#define NO_OF_DATA_MSGS         50
#define NO_OF_ITERS             3
#define ITER_DELAY_MS           1000
#define RX_TASK_PRIO            8
#define TX_TASK_PRIO            9
#define CTRL_TSK_PRIO           10
#define TX_GPIO_NUM             32
#define RX_GPIO_NUM             33
#define EXAMPLE_TAG             "TWAI Master"

#define ID_MASTER_STOP_CMD      0x0A0
#define ID_MASTER_START_CMD     0x0A1
#define ID_MASTER_PING          0x0A2
#define ID_SLAVE_STOP_RESP      0x0B0
#define ID_SLAVE_DATA           0x0B1
#define ID_SLAVE_PING_RESP      0x0B2

typedef enum {
    TX_SEND_PINGS,
    TX_SEND_START_CMD,
    TX_SEND_STOP_CMD,
    TX_TASK_EXIT,
} tx_task_action_t;

typedef enum {
    RX_RECEIVE_PING_RESP,
    RX_RECEIVE_DATA,
    RX_RECEIVE_STOP_RESP,
    RX_TASK_EXIT,
} rx_task_action_t;

static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_25KBITS();
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);


static const twai_message_t buttons_state = {
    // Message type and format settings
    .extd = 0,              // Standard Format message (11-bit ID)
    .rtr = 0,               // Send a data frame
    .ss = 0,                // Not single shot
    .self = 0,              // Not a self reception request
    .dlc_non_comp = 0,      // DLC is less than 8
    // Message ID and payload
    .identifier = ID_MASTER_START_CMD,
    .data_length_code = 2,
    .data = {0},
};


static QueueHandle_t tx_task_queue;
static QueueHandle_t rx_task_queue;
static SemaphoreHandle_t stop_ping_sem;
static SemaphoreHandle_t ctrl_task_sem;


/**************** Tasks Declaration  **********************/


void vSpiTask(void *pvParameters)
{
	esp_err_t ret;
	BaseType_t queue_status;
	uint16_t action=0x1;

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
	  switch_buttons[0]=0xAA;

	  if(*((uint16_t *)switch_buttons) != 0x0)
	  {

	    ESP_LOGI(TAG,"bottons pressed =%x %x",switch_buttons[0], switch_buttons[1]);
	    while (uxQueueSpacesAvailable(tx_task_queue) == 0)
	    {
	      ESP_LOGI(TAG,"Queue full");
	      vTaskDelay(500 / portTICK_PERIOD_MS);
	    }
	    if(pdTRUE == xQueueSendToBack(tx_task_queue, &action, portMAX_DELAY))
	      ESP_LOGI(TAG,"Queue Posted Action = %d",action);
	    else
	      ESP_LOGI(TAG,"Queue not Posted");
	    action++;
	  }
	}
}

/********************************************************/

static void twai_transmit_task(void *arg)
{

  esp_err_t error;

  while (1) {
        uint16_t action;
        while(xQueueReceive(tx_task_queue, &action, portMAX_DELAY) != pdTRUE);
        ESP_LOGI(EXAMPLE_TAG, "ACTION = %x", action);
        error = twai_transmit(&buttons_state, portMAX_DELAY);
        ESP_LOGI(EXAMPLE_TAG, "Transmitted status = %d", error);
    }
    vTaskDelete(NULL);
}

/********************************************************/

static void twai_receive_task(void *arg)
{
    while (1) {
        rx_task_action_t action;
        xQueueReceive(rx_task_queue, &action, portMAX_DELAY);

        if (action == RX_RECEIVE_PING_RESP) {
            //Listen for ping response from slave
            while (1) {
                twai_message_t rx_msg;
                twai_receive(&rx_msg, portMAX_DELAY);
                if (rx_msg.identifier == ID_SLAVE_PING_RESP) {
                    xSemaphoreGive(stop_ping_sem);
                    xSemaphoreGive(ctrl_task_sem);
                    break;
                }
            }
        } else if (action == RX_RECEIVE_DATA) {
            //Receive data messages from slave
            uint32_t data_msgs_rec = 0;
            while (data_msgs_rec < NO_OF_DATA_MSGS) {
                twai_message_t rx_msg;
                twai_receive(&rx_msg, portMAX_DELAY);
                if (rx_msg.identifier == ID_SLAVE_DATA) {
                    uint32_t data = 0;
                    for (int i = 0; i < rx_msg.data_length_code; i++) {
                        data |= (rx_msg.data[i] << (i * 8));
                    }
                    ESP_LOGI(EXAMPLE_TAG, "Received data value %"PRIu32, data);
                    data_msgs_rec ++;
                }
            }
            xSemaphoreGive(ctrl_task_sem);
        } else if (action == RX_RECEIVE_STOP_RESP) {
            //Listen for stop response from slave
            while (1) {
                twai_message_t rx_msg;
                twai_receive(&rx_msg, portMAX_DELAY);
                if (rx_msg.identifier == ID_SLAVE_STOP_RESP) {
                    xSemaphoreGive(ctrl_task_sem);
                    break;
                }
            }
        } else if (action == RX_TASK_EXIT) {
            break;
        }
    }
    vTaskDelete(NULL);
}


/***************************************/

void Configure_Can_Module()
{

  rx_task_queue = xQueueCreate(1, sizeof(rx_task_action_t));
  tx_task_queue = xQueueCreate(1, sizeof(tx_task_action_t));

  //xTaskCreatePinnedToCore(twai_receive_task, "TWAI_rx", 4096, NULL, RX_TASK_PRIO, NULL, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(twai_transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);

  //Install TWAI driver
  g_config.alerts_enable = TWAI_ALERT_ALL;
  ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
  ESP_LOGI(EXAMPLE_TAG, "Driver installed");
  ESP_ERROR_CHECK(twai_start());
  ESP_LOGI(EXAMPLE_TAG, "Driver started");

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

  Configure_Can_Module();

  xTaskCreate(vSpiTask, "buttons_task", 2048, (void *)0, 4, NULL);

}




