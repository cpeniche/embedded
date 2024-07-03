
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "motors.h"
#include "dictionary.h"

static const char *TAG = "Dictionary_Task";

void vprvGetCastedData(void *vpprvElement, eDataType eprvDataType, void *vpprvDest);
int8_t iprvDictionaryGetNextQueueElement(void **ppvprvData, int *piprvLength);
int8_t iprvDictionaryLookUpMessage(uint8_t uprvMessageId);

stQueueList *xpQueue=NULL;
stQueueList *xpQueueIndex=NULL;
stQueueList *xpProcessDataIndex=NULL;

SemaphoreHandle_t xQueueSemaphore;
StaticSemaphore_t xSemaphoreBuffer;


//stDictionary xpDictionaryEntry={0};
stDictionary xDictionary[configDICTIONARYSIZE]=
{
  {.uMessageId=0x44,.eDataType=eUNSIGNED16,.pvData=NULL,.vCallback=NULL},
  {.uMessageId=0x55,.eDataType=eUNSIGNED16,.pvData=&uButtons,.vCallback=vMotorsCallBack},
  {.uMessageId=DELIMITER}
};

SemaphoreHandle_t xDictionaryCreateQueueSemaphore()
{

  /* Create a mutex without using any dynamic memory allocation. */
  xQueueSemaphore = xSemaphoreCreateMutexStatic( &xSemaphoreBuffer );

  return  xQueueSemaphore;
}


/******************* xpCreateNode *************************/
stQueueList* xpCreateNode()
{

  stQueueList *xprvNewNode=NULL;

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
      if(xpQueue == NULL)
        return -1;
    
    /* Update Indexes */    
    xpQueueIndex=xpQueue;
    xpProcessDataIndex=xpQueue;    
  }
  else{

    /* Create a new Node*/
    xpQueueIndex->xpNextElement=xpCreateNode();
    if(xpQueueIndex->xpNextElement == NULL)
        return -1;

    /* Link the new node to the list*/    
    xpQueueIndex = xpQueueIndex->xpNextElement;    
  }
  /* Copy Data to the new Element */
  xpQueueIndex->vpQueueData=malloc(iprvLength*sizeof(uint8_t));
  if (xpQueueIndex->vpQueueData == NULL)
    return -1;
  memcpy(xpQueueIndex->vpQueueData,pvprvData,iprvLength);
  xpQueueIndex->xlength=iprvLength;

  return 0;
}


/***************** uAddDataToQueue ***************************/
int8_t iprvDictionaryGetNextQueueElement(void **ppvprvData, int *piprvLength)
{
  /*First In First out*/

  stQueueList *xprvItemToRemove=NULL;

  /* No Element in Queue*/
  if(xpQueue == NULL)
    return -1;
 
  *ppvprvData = malloc(xpQueueIndex->xlength * sizeof(uint8_t));
  if(*ppvprvData == NULL)
    return -1;
  /* Copy Element*/
  memcpy(*ppvprvData,xpProcessDataIndex->vpQueueData,xpQueueIndex->xlength);
  free(xpProcessDataIndex->vpQueueData);
  xpProcessDataIndex->vpQueueData = NULL;
  *piprvLength=xpQueueIndex->xlength;
  xprvItemToRemove=xpProcessDataIndex;

  /*Point to next element*/
  if(xpProcessDataIndex->xpNextElement != NULL)
    xpProcessDataIndex=xpProcessDataIndex->xpNextElement;

  /*Remove Processed Element from Queue*/
  free(xprvItemToRemove);
  if(xprvItemToRemove == xpQueue)
    xpQueue=NULL;
  xprvItemToRemove=NULL;
  
  return 0;
}

/*********************************************** */
void vProcessReceivedDataTask(void *pvParameters)
{

  void *vpprvQueueElement=NULL;
  int iprvLength;
  int iprvStatus;
  int8_t iprvIndexInDictionary=0;

  xDictionaryCreateQueueSemaphore();
  ESP_LOGI(TAG, "Data Process Task Started  ");

  while(1)
  {
    
    /* lookc for Message in Queue */
    while (1)
    {
      
      /* Get Semaphore */
      if (xSemaphoreTake(xQueueSemaphore,10 ) == pdTRUE)
      {
        iprvStatus = iprvDictionaryGetNextQueueElement(&vpprvQueueElement, &iprvLength);
        xSemaphoreGive(xQueueSemaphore);
        /* If there is data to process */
        if(iprvStatus != -1) 
        { 
          /* First byte is the Message ID*/
          iprvIndexInDictionary = iprvDictionaryLookUpMessage(((uint8_t *)vpprvQueueElement)[0]);
          if(iprvIndexInDictionary!=-1)
          {
            if(xDictionary[iprvIndexInDictionary].pvData != NULL)
              vprvGetCastedData(vpprvQueueElement, 
                             xDictionary[iprvIndexInDictionary].eDataType,
                             xDictionary[iprvIndexInDictionary].pvData);

            if(xDictionary[iprvIndexInDictionary].vCallback != NULL)
              xDictionary[iprvIndexInDictionary].vCallback(((uint8_t *)vpprvQueueElement)[1]);                                 
          }        
        }                
      }
    }
  }
}

/************************* prvDictionaryLookUpMessage ****************************** */
int8_t iprvDictionaryLookUpMessage(uint8_t uprvMessageId)
{

  int8_t uprvIndex=0;
  
  /* First byte is the Message ID*/
  while(xDictionary[uprvIndex].uMessageId != DELIMITER)
  {

    if(xDictionary[uprvIndex].uMessageId == uprvMessageId)
    {
      return uprvIndex;
    }
    uprvIndex++;
  }
  return -1;
}

/********************* vprvGetCastedData ********************* */
void vprvGetCastedData(void *vpprvElement, eDataType eprvDataType, void *vpprvDest)
{


  void *vpprvDataTypePosition = (void *)&(((uint8_t *)vpprvElement)[3]);

  switch(eprvDataType)
  { 
    case eUNSIGEND8:
      *((uint8_t*)vpprvDest)= *((uint8_t*)vpprvDataTypePosition);
      break;

    case eSIGNED8:
       *((int8_t*)vpprvDest)= *((int8_t*)vpprvDataTypePosition);
      break;

    case eUNSIGNED16:
       *((uint16_t*)vpprvDest)= *((uint16_t*)vpprvDataTypePosition);
      break;    

    default:
  }
}
