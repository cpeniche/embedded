#include <esp_log.h>
#include <string.h>
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

static const char *TAG = "Motors_Task";
static const char *MOVINGUP = "MOVING UP\0";
static const char *MOVINGDOWN = "MOVING DOWN\0";
static const char *MOVINGLEFT = "MOVING LEFT\0";
static const char *MOVINGRIGHT = "MOVING RIGHT\0";
static const char *NOMOVE     = "NO MOVE\0";

/* The variable used to hold the queue's data structure. */
static StaticQueue_t xMotorsStaticQueue;

/* The array to use as the queue's storage area. This must be at least
(uxQueueLength * uxItemSize) bytes. */
uint8_t ucMotorQueueStorageArea[ configMOTORQUEUESIZE * configITEMSIZE ];
QueueHandle_t xMotorQueue;


void vprvLatchMotorDriver();
void vprvWindowMotorDriver();
void vprvMirrorMotorDriver();

void (*vMotorsFunctions[3])(void)=
{
  vprvLatchMotorDriver,
  vprvWindowMotorDriver,
  vprvMirrorMotorDriver
};


gpio_config_t xprvMotorsIOConfiguration = {
  .intr_type = GPIO_INTR_DISABLE,
  .mode = GPIO_MODE_OUTPUT,
  .pin_bit_mask = 1<<LATCHMOTORPHASE | 1<< LATCHMOTORENABLE | 1<<WINDOWMOTOR_INA | 1<<WINDOWMOTOR_INB | \
                  1<<WINDOWMOTOR_ENA | 1<< WINDOWMOTOR_ENB | 1<< WINDOWMOTOR_PWM | 1<<TLEMOTORENABLE,
                
  .pull_down_en = 1,
  .pull_up_en = 0
};

uint16_t uButtons;
eMOTOR_TYPE eMotorFunctionSelect=eNOMOTOR;
spi_transaction_t prvxSpiTransaction;
xTleTxMessageType xTLETxMessage;
xTleRxMessageType xTLERxMessage;


void vMotorsTask(void *pvParameters)
{
 

  /* Create a queue capable of containing 10 uint64_t values. */
  xMotorQueue = xQueueCreateStatic( configMOTORQUEUESIZE,
                                configITEMSIZE,
                                ucMotorQueueStorageArea,
                                &xMotorsStaticQueue );

  /* ucMotorQueueStorageArea was not NULL so xMotorQueue should not be NULL. */
  configASSERT( xMotorQueue );                                

  gpio_config(&xprvMotorsIOConfiguration);
  gpio_set_level(LATCHMOTORPHASE,  0);
  gpio_set_level(LATCHMOTORENABLE, 0);
  //gpio_set_level(TLEMOTORCHIPSELECT, 1);
  gpio_set_level(TLEMOTORENABLE, 1);


  memset(&prvxSpiTransaction, 0, sizeof(prvxSpiTransaction));       //Zero out the transaction
  prvxSpiTransaction.length=16;                    //Command is 8 bits
  prvxSpiTransaction.tx_buffer=&xTLETxMessage;    //The data is the cmd itself
  prvxSpiTransaction.rx_buffer=&xTLERxMessage;
  prvxSpiTransaction.user=(void*)0;    
  prvxSpiTransaction.rxlength=16;
  

  ESP_LOGI(TAG, "Motors Task Started  ");

  while(1)
  {
      /*Wait for a Message from the Buttons Callback*/
      if(xQueueReceive(xMotorQueue,&eMotorFunctionSelect,portMAX_DELAY) == pdPASS)
      {
        if(NULL != vMotorsFunctions[eMotorFunctionSelect])
          vMotorsFunctions[eMotorFunctionSelect]();
      }
  }
}

void vprvLatchMotorDriver()
{

  ESP_LOGI(TAG, " %s Executed",__func__);
  if(uButtons & DOOR_CLOSE )
    gpio_set_level(LATCHMOTORPHASE,  0);
  else
    gpio_set_level(LATCHMOTORPHASE,  1);
  
  gpio_set_level(LATCHMOTORENABLE, 1);
  vTaskDelay(pdMS_TO_TICKS(100));
  gpio_set_level(LATCHMOTORENABLE, 0);

}

void vprvWindowMotorDriver()
{
  
  ESP_LOGI(TAG, " %s Executed",__func__);

  if(LEFT_WINDOW_UP & uButtons)
  {
    gpio_set_level(WINDOWMOTOR_INA,  0);
    gpio_set_level(WINDOWMOTOR_INB,  1);
  }
  if(LEFT_WINDOW_DOWN & uButtons)
  {
    gpio_set_level(WINDOWMOTOR_INA,  1);
    gpio_set_level(WINDOWMOTOR_INB,  0);
  }
  gpio_set_level(WINDOWMOTOR_PWM,  1);
}

void vprvMirrorMotorDriver()
{
  esp_err_t pxReturnCode;  
  uint8_t uprvMessage[10];
  uint8_t uprvtest;

  xTLETxMessage.NA=1;  
  xTLETxMessage.uAddress = CONTROLREGISTER;
  xTLETxMessage.uLabt = 1;
  xTLETxMessage.uAccesType = 1;

  if((MIRROR_SELECT_LEFT & uButtons) == MIRROR_SELECT_LEFT)
  {
    memset(&uprvMessage,0x0,sizeof(uprvMessage));
    uButtons &= (~MIRROR_SELECT_LEFT);
    xTLETxMessage.uData = 0x0;
    memcpy(&uprvMessage,NOMOVE, strlen(NOMOVE));

    if((MIRROR_MOVE_UP & uButtons) == MIRROR_MOVE_UP)
    {
      xTLETxMessage.uData = MIRROR_DRIVER_MOVE_UP; 
      memcpy(&uprvMessage,MOVINGUP, strlen(MOVINGUP));
    }
    if((MIRROR_MOVE_DOWN & uButtons) == MIRROR_MOVE_DOWN)
    {
      xTLETxMessage.uData = MIRROR_DRIVER_MOVE_DOWN; 
      memcpy(&uprvMessage,MOVINGDOWN, strlen(MOVINGDOWN));     
    }
    if((MIRROR_MOVE_LEFT & uButtons)== MIRROR_MOVE_LEFT )
    {
      xTLETxMessage.uData = MIRROR_DRIVER_MOVE_LEFT;
      memcpy(&uprvMessage,MOVINGLEFT, strlen(MOVINGLEFT));      
    }
    if((MIRROR_MOVE_RIGHT & uButtons) == MIRROR_MOVE_RIGHT)
    {
      xTLETxMessage.uData = MIRROR_DRIVER_MOVE_RIGHT; 
      memcpy(&uprvMessage,MOVINGRIGHT, strlen(MOVINGRIGHT)); 
    }
  
    uprvtest = xTLETxMessage.uData;         
    gpio_set_level(TLEMOTORCHIPSELECT, 0);
    pxReturnCode =  spi_device_polling_transmit(spi,&prvxSpiTransaction);
    gpio_set_level(TLEMOTORCHIPSELECT, 1);

    ESP_LOGI(TAG, "%s : %x",uprvMessage,uprvtest);            
  
    assert(pxReturnCode==ESP_OK);    
  }
  

  //ESP_LOGI(TAG, " %s Executed",__func__);
}