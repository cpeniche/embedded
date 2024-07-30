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
#include "spidrivermodel.h"
#include "dictionary.h"
#include "buttons.h"
#include "motors.h"
#include "drivers/include/espspi.h"
#include "main.h"

static const char *TAG = "Motors_Task";
static const char *MOVINGUP = "MOVING UP\0";
static const char *MOVINGDOWN = "MOVING DOWN\0";
static const char *MOVINGLEFT = "MOVING LEFT\0";
static const char *MOVINGRIGHT = "MOVING RIGHT\0";
static const char *NOMOVE = "NO MOVE\0";

/* The variable used to hold the queue's data structure. */
static StaticQueue_t xMotorsStaticQueue;
uint16_t uButtons;

/* The array to use as the queue's storage area. This must be at least
(uxQueueLength * uxItemSize) bytes. */
uint8_t ucMotorQueueStorageArea[configMOTORQUEUESIZE * configITEMSIZE];
QueueHandle_t xMotorQueue;

void vprvLatchMotorDriver(Spi *);
void vprvWindowMotorDriver(Spi *);
void vprvMirrorMotorDriver(Spi *);

void (*vMotorsFunctions[3])(Spi *) =
    {
        vprvLatchMotorDriver,
        vprvWindowMotorDriver,
        vprvMirrorMotorDriver};

gpio_config_t xprvMotorsIOConfiguration = {

    .pin_bit_mask = 1 << LATCHMOTORPHASE | 1 << LATCHMOTORENABLE | 1 << WINDOWMOTOR_INA | 1 << WINDOWMOTOR_INB |
                    1 << WINDOWMOTOR_ENA | 1 << WINDOWMOTOR_ENB | 1 << WINDOWMOTOR_PWM | 1 << TLEMOTORENABLE,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .intr_type = GPIO_INTR_DISABLE,

};

eMOTOR_TYPE eMotorFunctionSelect = eNOMOTOR;
spi_transaction_t prvxSpiTransaction;
xTleTxMessageType xTLETxMessage;
xTleRxMessageType xTLERxMessage;

void vMotorsTask(void *pvParameters)
{

  esp_err_t pxReturnCode = ESP_OK;
  /* Create a queue capable of containing 10 uint64_t values. */
  xMotorQueue = xQueueCreateStatic(configMOTORQUEUESIZE,
                                   configITEMSIZE,
                                   ucMotorQueueStorageArea,
                                   &xMotorsStaticQueue);

  /* ucMotorQueueStorageArea was not NULL so xMotorQueue should not be NULL. */
  configASSERT(xMotorQueue);

  gpio_config(&xprvMotorsIOConfiguration);
  gpio_set_level(LATCHMOTORPHASE, 0);
  gpio_set_level(LATCHMOTORENABLE, 0);
  gpio_set_level(TLEMOTORENABLE, 1);

  SpiBuilder *xprvSpiBuilder = new EspSpiBuilder(reinterpret_cast<uint8_t *>(&(xTLETxMessage)),
                                                 reinterpret_cast<uint8_t *>(&(xTLERxMessage)));
  Spi *xprvSpi = xprvSpiBuilder->xBuild();
  delete xprvSpiBuilder;

  xprvSpi->Init();
  pxReturnCode = *(static_cast<esp_err_t *>(xprvSpi->GetError()));
  if (pxReturnCode != ESP_OK)
  {
    ESP_LOGE(TAG, "Spi Initialization Error : %d", pxReturnCode);
    assert(pxReturnCode == ESP_OK);
  }

  ESP_LOGI(TAG, "Motors Task Started  ");

  while (1)
  {
    /*Wait for a Message from the Buttons Callback*/
    if (xQueueReceive(xMotorQueue, &eMotorFunctionSelect, portMAX_DELAY) == pdPASS)
    {
      if (NULL != vMotorsFunctions[eMotorFunctionSelect])
        vMotorsFunctions[eMotorFunctionSelect](xprvSpi);
    }
  }
}

void vprvLatchMotorDriver(Spi *vpArgs)
{

  ESP_LOGI(TAG, " %s Executed", __func__);
  if (uButtons & DOOR_CLOSE)
    gpio_set_level(LATCHMOTORPHASE, 0);
  else
    gpio_set_level(LATCHMOTORPHASE, 1);

  gpio_set_level(LATCHMOTORENABLE, 1);
  vTaskDelay(pdMS_TO_TICKS(100));
  gpio_set_level(LATCHMOTORENABLE, 0);
}

void vprvWindowMotorDriver(Spi *vpArgs)
{

  ESP_LOGI(TAG, " %s Executed", __func__);

  if (LEFT_WINDOW_UP & uButtons)
  {
    gpio_set_level(WINDOWMOTOR_INA, 0);
    gpio_set_level(WINDOWMOTOR_INB, 1);
  }
  if (LEFT_WINDOW_DOWN & uButtons)
  {
    gpio_set_level(WINDOWMOTOR_INA, 1);
    gpio_set_level(WINDOWMOTOR_INB, 0);
  }
  gpio_set_level(WINDOWMOTOR_PWM, 1);
}

void vprvMirrorMotorDriver(Spi *vpArgs)
{
  esp_err_t pxReturnCode;
  uint8_t uprvMessage[12];
  uint8_t uprvtest;
  Spi *xprvSpi = vpArgs;

  xTLETxMessage.NA = 1;
  xTLETxMessage.uAddress = CONTROLREGISTER;
  xTLETxMessage.uLabt = 1;
  xTLETxMessage.uAccesType = 1;

  if ((MIRROR_SELECT_LEFT & uButtons) == MIRROR_SELECT_LEFT)
  {
    memset(&uprvMessage, 0x0, sizeof(uprvMessage));
    uButtons &= (~MIRROR_SELECT_LEFT);
    xTLETxMessage.uData = 0x0;
    memcpy(&uprvMessage, NOMOVE, strlen(NOMOVE));
    gpio_set_level(TLEMOTORENABLE, 1);

    if ((MIRROR_MOVE_UP & uButtons) == MIRROR_MOVE_UP)
    {
      xTLETxMessage.uData = MIRROR_DRIVER_MOVE_UP;
      memcpy(&uprvMessage, MOVINGUP, strlen(MOVINGUP));
    }
    if ((MIRROR_MOVE_DOWN & uButtons) == MIRROR_MOVE_DOWN)
    {
      xTLETxMessage.uData = MIRROR_DRIVER_MOVE_DOWN;
      memcpy(&uprvMessage, MOVINGDOWN, strlen(MOVINGDOWN));
    }
    if ((MIRROR_MOVE_LEFT & uButtons) == MIRROR_MOVE_LEFT)
    {
      xTLETxMessage.uData = MIRROR_DRIVER_MOVE_LEFT;
      memcpy(&uprvMessage, MOVINGLEFT, strlen(MOVINGLEFT));
    }
    if ((MIRROR_MOVE_RIGHT & uButtons) == MIRROR_MOVE_RIGHT)
    {
      xTLETxMessage.uData = MIRROR_DRIVER_MOVE_RIGHT;
      memcpy(&uprvMessage, MOVINGRIGHT, strlen(MOVINGRIGHT));
    }

    uprvtest = xTLETxMessage.uData;
    //gpio_set_level(TLEMOTORCHIPSELECT, 0);

    xprvSpi->Transmit();
    pxReturnCode = *(static_cast<esp_err_t *>(xprvSpi->GetError()));
    assert(pxReturnCode == ESP_OK);

    //gpio_set_level(TLEMOTORCHIPSELECT, 1);

    ESP_LOGI(TAG, "%s : %x", uprvMessage, uprvtest);

    assert(pxReturnCode == ESP_OK);
  }

  // ESP_LOGI(TAG, " %s Executed",__func__);
}



void vESPNowMessageProcessButtonsCallBack(uint8_t uprvRW, eDataTYPE xprvDataType, void *uprvParameters)
{

  eMOTOR_TYPE xprivItemToQueue;

  vGetCastedData(uprvParameters, xprvDataType, &uButtons);

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