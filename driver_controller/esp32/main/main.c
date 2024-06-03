/*
 * SPDX-FileCopyrightText: 2010-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

/*
 * The following example demonstrates a master node in a TWAI network. The master
 * node is responsible for initiating and stopping the transfer of data messages.
 * The example will execute multiple iterations, with each iteration the master
 * node will do the following:
 * 1) Start the TWAI driver
 * 2) Repeatedly send ping messages until a ping response from slave is received
 * 3) Send start command to slave and receive data messages from slave
 * 4) Send stop command to slave and wait for stop response from slave
 * 5) Stop the TWAI driver
 */
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

/* lcd panel includes*/
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_err.h"
#include "lvgl.h"


extern void Init_Display();

/* --------------------- SPI Definitions and static variables ------------------ */

#define LCD_HOST    HSPI_HOST
#define PARALLEL_LOAD 25
#define PIN_NUM_CLK   26
#define PIN_NUM_MISO  27
#define PARALLEL_LINES 16
//static const char *TAG = "Spi_Task";

#if 0
gpio_config_t io_conf = {
    .intr_type = GPIO_INTR_DISABLE,
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = 1<<PARALLEL_LOAD,
    .pull_down_en = 0,
    .pull_up_en = 0
};

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

/* --------------------- TWAI Definitions and static variables ------------------ */
//Example Configuration
#define PING_PERIOD_MS          250
#define NO_OF_DATA_MSGS         50
#define NO_OF_ITERS             3
#define ITER_DELAY_MS           1000
#define RX_TASK_PRIO            8
#define TX_TASK_PRIO            9
#define CTRL_TSK_PRIO           10
#define TX_GPIO_NUM             21
#define RX_GPIO_NUM             22
#define EXAMPLE_TAG             "TWAI Master"

#define ID_MASTER_STOP_CMD      0x0A0
#define ID_MASTER_START_CMD     0x0A1
#define ID_MASTER_PING          0x0A2
#define ID_SLAVE_STOP_RESP      0x0B0
#define ID_SLAVE_DATA           0x0B1
#define ID_SLAVE_PING_RESP      0x0B2

typedef enum {
    TX_SEND_BUTTONS,

} tx_task_action_t;

typedef enum {
    RX_RECEIVE_PING_RESP,
    RX_RECEIVE_DATA,
    RX_RECEIVE_STOP_RESP,
    RX_TASK_EXIT,
} rx_task_action_t;

static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);

static twai_message_t buttons_message = {
    // Message type and format settings
    .extd = 0,              // Standard Format message (11-bit ID)
    .rtr = 0,               // Send a data frame
    .ss = 1,                // Is single shot (won't retry on error or NACK)
    .self = 0,              // Not a self reception request
    .dlc_non_comp = 0,      // DLC is less than 8
    // Message ID and payload
    .identifier = ID_MASTER_PING,
    .data_length_code = 2,
    .data = {0},
};

static QueueHandle_t tx_task_queue;
static QueueHandle_t rx_task_queue;

/* --------------------------- Tasks and Functions -------------------------- */

void vSpiTask(void *pvParameters)
{
  esp_err_t ret;
  tx_task_action_t tx_action=TX_SEND_BUTTONS;

  gpio_config(&io_conf);
  gpio_set_level(PARALLEL_LOAD, 1);

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

  //xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);

  while(1)
  {
    gpio_set_level(PARALLEL_LOAD, 0);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    gpio_set_level(PARALLEL_LOAD, 1);
    ret =  spi_device_polling_transmit(spi,&t);
    assert(ret==ESP_OK);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    buttons_message.data[0]=0XAA;
    buttons_message.data[1]=0XBB;
    xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
  }
}

/*****************************************************************/

static void twai_receive_task(void *arg)
{
  rx_task_action_t action;
  twai_message_t rx_msg;

  while (1)
  {
    xQueueReceive(rx_task_queue, &action, portMAX_DELAY);
    twai_receive(&rx_msg, portMAX_DELAY);
  }
}

/**********************************************************/
static void twai_transmit_task(void *arg)
{
  tx_task_action_t action;
  while (1) {

    xQueueReceive(tx_task_queue, &action, portMAX_DELAY);
    ESP_LOGI(EXAMPLE_TAG, "Transmitting ping");
    twai_transmit(&buttons_message, portMAX_DELAY);
  }
}


/*************************************************************************/
void Configure_Can_Module()
{

  //Create tasks, queues, and semaphores

  rx_task_queue = xQueueCreate(1, sizeof(rx_task_action_t));
  tx_task_queue = xQueueCreate(1, sizeof(tx_task_action_t));

  xTaskCreatePinnedToCore(twai_receive_task, "TWAI_rx", 4096, NULL, RX_TASK_PRIO, NULL, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(twai_transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);

  //Install TWAI driver
  ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
  ESP_LOGI(EXAMPLE_TAG, "Driver installed");
  ESP_ERROR_CHECK(twai_start());
  ESP_LOGI(EXAMPLE_TAG, "Driver started");

}
#endif

void app_main(void)
{

  //Configure_Can_Module();
  //xTaskCreate(vSpiTask, "buttons_task", 2048, (void *)0, 4, NULL);

  Init_Display();
  while(1)
  {
  // raise the task priority of LVGL and/or reduce the handler period can improve the performance
  vTaskDelay(pdMS_TO_TICKS(10));
  // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
  lv_timer_handler();
  }

}

