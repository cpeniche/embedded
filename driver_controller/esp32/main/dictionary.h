#pragma once


typedef struct 
{
  uint8_t uMessageId;
  uint8_t uButtonsState[2];
  
}stButtonsMessage;

typedef struct Queue
{
  struct Queue *xpHead;
  struct Queue *xpNextElement;
  stButtonsMessage xData;

}stQueueList;


extern SemaphoreHandle_t xDictionaryCreateQueueSemaphore();
extern int8_t iDictionaryAddDataToQueue(void * data);
extern int8_t iDictionaryGetNextQueueElement(void * data);
extern SemaphoreHandle_t xQueueSemaphore;


