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

//#define USE_CAN 0

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

#ifdef USE_CAN
#include "driver/twai.h"
#include "soc/twai_periph.h"    // For GPIO matrix signal index
#endif 




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

#ifdef USE_CAN
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

#endif
/* --------------------------- Tasks and Functions -------------------------- */


EventGroupHandle_t xESPnowEventGroupHandle;

/* Declare a variable to hold the data associated with the created event group. */
StaticEventGroup_t xStaticCreatedEventGroup;

/* --------------------------- Events -------------------------*/


/*****************************************************************/
#ifdef USE_CAN
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
 /* Create statically event group for ESPnow Events*/
 xESPnowEventGroupHandle = xEventGroupCreateStatic( &xStaticCreatedEventGroup );

 #ifdef USE_CAN
  Configure_Can_Module();
#endif
/* Start ESP Now configuration */
  vWifiConfigureESPNow();

  /* Create Message Receive Task */
  xTaskCreate(vProcessReceivedDataTask, 
              "ProcessReceivedData_task",                     
              4096, 
              (void *)0, 
              tskIDLE_PRIORITY, 
              NULL);

  /* Create Button Read spi task */
  xTaskCreate(vButtonsTask, 
              "buttons_task",                     
              BUTTON_TASK_STACK_SIZE, 
              (void *)0, 
              tskIDLE_PRIORITY, 
              NULL);
  /* Create Motor task for latch, windows and mirrors*/
  xTaskCreate(vMotorsTask, 
              "Motors_task",                     
              4096, 
              (void *)0, 
              tskIDLE_PRIORITY, 
              NULL);              
}

