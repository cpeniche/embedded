
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "dictionary.h"


stQueueList *xpQueue=NULL;
stQueueList *xpQueueIndex=NULL;
stQueueList *xpProcessDataIndex=NULL;

SemaphoreHandle_t xQueueSemaphore;
StaticSemaphore_t xSemaphoreBuffer;


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
int8_t iDictionaryAddDataToQueue(void * data)
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

    /* Copy Data to the new Element */
    xpQueueIndex->xData=*((stButtonsMessage *)data);
  }
  else{

    /* Create a new Node*/
    xpQueueIndex->xpNextElement=xpCreateNode();
    if(xpQueueIndex->xpNextElement == NULL)
        return -1;

    /* Link the new node to the list*/    
    xpQueueIndex = xpQueueIndex->xpNextElement;

     /* Copy Data to the new Element */
    xpQueueIndex->xData=*((stButtonsMessage *)data);
  }

  return 0;
}


/***************** uAddDataToQueue ***************************/
int8_t iDictionaryGetNextQueueElement(void * data)
{
  /*First In First out*/

  stQueueList *xprvItemToRemove=NULL;

  /* No Element in Queue*/
  if(xpQueue == NULL)
    return -1;

  /* Copy Element*/
  *((stButtonsMessage *)data) =  xpProcessDataIndex->xData;

  xprvItemToRemove=xpProcessDataIndex;

  /*Point to next element*/
  if(xpProcessDataIndex->xpNextElement != NULL)
    xpProcessDataIndex=xpProcessDataIndex->xpNextElement;

  /*Remove Processed Element from Queue*/
  free(xprvItemToRemove);

  return 0;
}