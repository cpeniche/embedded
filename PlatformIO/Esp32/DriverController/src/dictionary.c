
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "driver/spi_master.h"
#include "motors.h"
#include "dictionary.h"
#include "main.h"

static const char *TAG = "Dictionary_Task";

void vprvGetCastedData(void *vpprvElement, eDataTYPE eprvDataType, void *vpprvDest);
int8_t iprvDictionaryGetNextQueueElement(void **ppvprvData, int *piprvLength);
int8_t iprvDictionaryLookUpMessage(uint8_t uprvMessageId);

stQueueList *xpQueue = NULL;
stQueueList *xpQueueIndex = NULL;
stQueueList *xpProcessDataIndex = NULL;

SemaphoreHandle_t xQueueSemaphore;
StaticSemaphore_t xSemaphoreBuffer;

stDictionary *xpDictionaryEntry = NULL;

stDictionary xDictionary[configDICTIONARYSIZE] =
    {
        {.uMessageId = 0x44, .eDataType = eUNSIGNED16, .vCallback = NULL},
        {.uMessageId = 0x55, .eDataType = eUNSIGNED16, .vCallback = vESPNowMessageProcessButtonsCallBack},
        {.uMessageId = DELIMITER}};

SemaphoreHandle_t xDictionaryCreateQueueSemaphore()
{

  /* Create a mutex without using any dynamic memory allocation. */
  xQueueSemaphore = xSemaphoreCreateMutexStatic(&xSemaphoreBuffer);

  return xQueueSemaphore;
}

/******************* xpCreateNode *************************/
stQueueList *xpCreateNode()
{

  stQueueList *xprvNewNode = NULL;

  xprvNewNode = malloc(sizeof(stQueueList));
  if (xprvNewNode != NULL)
    xprvNewNode->xpNextElement = NULL;

  return xprvNewNode;
}

/***************** uAddDataToQueue ***************************/
int8_t iDictionaryAddDataToQueue(void *pvprvData, int iprvLength)
{

  /* If no elements, Create one node and point
     indexes to first element
  */
  if (xpQueue == NULL)
  {
    /* Create Element*/
    xpQueue = xpCreateNode();
    if (xpQueue == NULL)
      return -1;

    /* Update Indexes */
    xpQueueIndex = xpQueue;
    xpProcessDataIndex = xpQueue;
  }
  else
  {

    /* Create a new Node*/
    xpQueueIndex->xpNextElement = xpCreateNode();
    if (xpQueueIndex->xpNextElement == NULL)
      return -1;

    /* Link the new node to the list*/
    xpQueueIndex = xpQueueIndex->xpNextElement;
  }
  /* Copy Data to the new Element */
  xpQueueIndex->vpQueueData = malloc(iprvLength * sizeof(uint8_t));
  if (xpQueueIndex->vpQueueData == NULL)
    return -1;
  memcpy(xpQueueIndex->vpQueueData, pvprvData, iprvLength);
  xpQueueIndex->xlength = iprvLength;

  return 0;
}

/***************** uAddDataToQueue ***************************/
int8_t iprvDictionaryGetNextQueueElement(void **ppvprvData, int *piprvLength)
{
  /*First In First out*/

  stQueueList *xprvItemToRemove = NULL;

  /* No Element in Queue*/
  if (xpQueue == NULL)
    return -1;

  *ppvprvData = xpProcessDataIndex->vpQueueData;

  *piprvLength = xpQueueIndex->xlength;

  /* Remove the node from the queue */
  xprvItemToRemove = xpProcessDataIndex;

  /*Point to next element*/
  if (xpProcessDataIndex->xpNextElement != NULL)
    xpProcessDataIndex = xpProcessDataIndex->xpNextElement;

  /*Remove Processed Element from Queue*/
  free(xprvItemToRemove);
  if (xprvItemToRemove == xpQueue)
    xpQueue = NULL;
  xprvItemToRemove = NULL;

  return 0;
}

/*********************************************** */
void vProcessReceivedDataTask(void *pvParameters)
{

  void *vpprvQueueElement = NULL;
  int iprvLength;
  int iprvStatus;
  int8_t iprvIndexInDictionary = 0;

  xDictionaryCreateQueueSemaphore();
  ESP_LOGI(TAG, "Data Process Task Started  ");

  while (1)
  {

    /* lookc for Message in Queue */
    while (1)
    {

      /* Get Semaphore */
      if (xSemaphoreTake(xQueueSemaphore, 10) == pdTRUE)
      {

        iprvStatus = iprvDictionaryGetNextQueueElement(&vpprvQueueElement, &iprvLength);
        xSemaphoreGive(xQueueSemaphore);
        /* If there is data to process */
        if (iprvStatus != -1)
        {
          /*Look up for the Message Id in Dictionary*/
          iprvIndexInDictionary = iprvDictionaryLookUpMessage(((stMessageHeader *)vpprvQueueElement)->uMessageId);

          if (iprvIndexInDictionary != -1)
          {

            ResetSleepTimer();

            /*Get the Dictionary Specifications*/
            xpDictionaryEntry = &xDictionary[iprvIndexInDictionary];

            /* Cast the received data according to the Data Type enum
            vprvGetCastedData(vpprvQueueElement,
                              xpDictionaryEntry->eDataType,
                              xpDictionaryEntry->pvData);*/

            /* If Callback defined, execute it*/
            if (xpDictionaryEntry->vCallback != NULL)
              xDictionary[iprvIndexInDictionary].vCallback(((stMessageHeader *)vpprvQueueElement)->uReadWrite,
                                                           xpDictionaryEntry->eDataType,
                                                           (vpprvQueueElement + sizeof(stMessageHeader)));

            /* free the memory where the received data was store*/
            free(vpprvQueueElement);
          }
        }
      }
    }
  }
}

/************************* prvDictionaryLookUpMessage ****************************** */
int8_t iprvDictionaryLookUpMessage(uint8_t uprvMessageId)
{

  int8_t uprvIndex = 0;

  /* First byte is the Message ID*/
  while (xDictionary[uprvIndex].uMessageId != DELIMITER)
  {

    /* Message id found, return index*/
    if (xDictionary[uprvIndex].uMessageId == uprvMessageId)
    {
      return uprvIndex;
    }
    uprvIndex++;
  }
  /* Message Id not found */
  return -1;
}

/********************* vprvGetCastedData ********************* */
void vGetCastedData(void *vpprvData, eDataTYPE eprvDataType, void *vpprvDest)
{

  switch (eprvDataType)
  {
  case eUNSIGEND8:
    *((uint8_t *)vpprvDest) = *((uint8_t *)vpprvData);
    break;

  case eSIGNED8:
    *((int8_t *)vpprvDest) = *((int8_t *)vpprvData);
    break;

  case eUNSIGNED16:
    *((uint16_t *)vpprvDest) = *((uint16_t *)vpprvData);
    break;

  default:
  }

  ESP_LOGI(TAG, "Data Received Value : %x", *((uint16_t *)vpprvDest));
}
