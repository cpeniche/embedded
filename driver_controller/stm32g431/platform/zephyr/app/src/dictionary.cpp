
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(dictionary, LOG_LEVEL_DBG);
#include <zephyr/kernel.h>
#include "dictionary.h"


#define PRIORITY K_PRIO_PREEMPT(15) // k_thread_priority_get(k_current_get())
#define STACK_SIZE 1024
#define SLEEP_PERIOD K_SECONDS(1)
Dictionary dictionaryInstance = Dictionary();

void vProcessReceivedDataTask(void);
/* Thread definitions*/
K_THREAD_DEFINE(ProcessData, STACK_SIZE, vProcessReceivedDataTask, NULL, NULL, NULL,
								PRIORITY, 0, 0);


/******************* CreateNode *************************/
stQueueList* Dictionary::CreateNode()
{

  stQueueList *xprvNewNode=NULL;

  xprvNewNode = static_cast<stQueueList*>(k_malloc(sizeof(stQueueList)));
  if (xprvNewNode != NULL)
    xprvNewNode->xpNextElement = NULL;

  return xprvNewNode;
}

/***************** uAddDataToQueue ***************************/
int8_t Dictionary::AddDataToQueue(void *pvprvData, int iprvLength)
{
  
  /* If no elements, Create one node and point
     indexes to first element
  */
  if (xpQueue == NULL)
  {
    /* Create Element*/
    xpQueue = CreateNode();
      if(xpQueue == NULL)
        return -1;
    
    /* Update Indexes */    
    xpQueueIndex=xpQueue;
    xpProcessDataIndex=xpQueue;    
  }
  else{

    /* Create a new Node*/
    xpQueueIndex->xpNextElement=CreateNode();
    if(xpQueueIndex->xpNextElement == NULL)
        return -1;

    /* Link the new node to the list*/    
    xpQueueIndex = xpQueueIndex->xpNextElement;    
  }
  /* Copy Data to the new Element */
  xpQueueIndex->vpQueueData=k_malloc(iprvLength*sizeof(uint8_t));
  if (xpQueueIndex->vpQueueData == NULL)
    return -1;
  memcpy(xpQueueIndex->vpQueueData,pvprvData,iprvLength);
  xpQueueIndex->xlength=iprvLength;

  return 0;
}


/***************** uAddDataToQueue ***************************/
int8_t Dictionary::GetNextQueueElement(void **ppvprvData, int *piprvLength)
{
  /*First In First out*/

  stQueueList *xprvItemToRemove=NULL;

  /* No Element in Queue*/
  if(xpQueue == NULL)
    return -1;
 
  *ppvprvData = xpProcessDataIndex->vpQueueData;
  
  *piprvLength=xpQueueIndex->xlength;

  /* Remove the node from the queue */
  xprvItemToRemove=xpProcessDataIndex;

  /*Point to next element*/
  if(xpProcessDataIndex->xpNextElement != NULL)
    xpProcessDataIndex=xpProcessDataIndex->xpNextElement;

  /*Remove Processed Element from Queue*/
  k_free(xprvItemToRemove);
  if(xprvItemToRemove == xpQueue)
    xpQueue=NULL;
  xprvItemToRemove=NULL;
  
  return 0;
}

/****************vProcessReceivedDataTask*********************/
void Dictionary::Task(void)
{

  void *vpprvQueueElement=NULL;
  int iprvLength;
  int iprvStatus;
  int8_t iprvIndexInDictionary=0;

  LOG_INF("Data Process Task Started  ");
    
  /* look for Message in Queue */
  while (1)
  {
    
    /* Get Semaphore */
    if (SemaphoreTake() == 0)
    {
              
      iprvStatus = GetNextQueueElement(&vpprvQueueElement, &iprvLength);
      SemaphoreGive();
      /* If there is data to process */
      if(iprvStatus != -1) 
      { 
        /*Look up for the Message Id in Dictionary*/          
        iprvIndexInDictionary = LookUpMessage(((stMessageHeader *)vpprvQueueElement)->uMessageId);
        
        if(iprvIndexInDictionary!=-1)
        {
                    
          /*Get the Dictionary Specifications*/
          xpDictionaryEntry=&xDictionary[iprvIndexInDictionary];

          /* Cast the received data according to the Data Type enum*/
          if(xpDictionaryEntry->pvData != NULL)
              GetCastedData(vpprvQueueElement, 
                            xpDictionaryEntry->enDataType,
                            xpDictionaryEntry->pvData);

          /* If Callback defined, execute it*/
          if(xpDictionaryEntry->vCallback != NULL)
            xDictionary[iprvIndexInDictionary].vCallback(((stMessageHeader *)vpprvQueueElement)->uReadWrite); 

          /* k_free the memory where the received data was store*/  
          k_free(vpprvQueueElement);                                
        }        
      }                
    }
    else
      LOG_INF("Could Not get Sempahore");
  }  
}

/************************* prvDictionaryLookUpMessage ****************************** */
int8_t Dictionary::LookUpMessage(uint8_t uprvMessageId)
{

  int8_t uprvIndex=0;
  
  /* First byte is the Message ID*/
  while(xDictionary[uprvIndex].uMessageId != DELIMITER)
  {

    /* Message id found, return index*/
    if(xDictionary[uprvIndex].uMessageId == uprvMessageId)
    {
      return uprvIndex;
    }
    uprvIndex++;
  }
  /* Message Id not found */
  return -1;
}

/********************* vprvGetCastedData ********************* */
void Dictionary::GetCastedData(void *vpprvElement, eDataType eprvDataType, void *vpprvDest)
{

  /*Point to the Data type element on the Message Received*/
  void *vpprvDataPosition = vpprvElement + sizeof(stMessageHeader);

  switch(eprvDataType)
  { 
    case eUNSIGEND8:
      *((uint8_t*)vpprvDest)= *((uint8_t*)vpprvDataPosition);
      break;

    case eSIGNED8:
       *((int8_t*)vpprvDest)= *((int8_t*)vpprvDataPosition);
      break;

    case eUNSIGNED16:
       *((uint16_t*)vpprvDest)= *((uint16_t*)vpprvDataPosition);
      break;    

    default:
      break;
  }

  LOG_INF("Data Received Value : %x", *((uint16_t*)vpprvDest));
}

/****************************************/
void vProcessReceivedDataTask(void)
{
    dictionaryInstance.Task();
}

Dictionary *getDictionaryInstance(void)
{
  return &dictionaryInstance;
}