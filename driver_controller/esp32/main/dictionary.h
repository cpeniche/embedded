#pragma once


#define configMAXARRAYSIZE 10
#define configDICTIONARYSIZE 5
#define DELIMITER 0xFF

typedef enum 
{
   eUNSIGEND8,
   eSIGNED8,
   eUNSIGNED16,
   eSIGNED16,
   eUNSIGNED32,
   eSIGNED32,
   eUNSIGNED64,
   eSIGNED64,
   eFLOAT
}eDataType;

typedef struct 
{
  uint8_t uMessageId;
  uint8_t uReadWrite;
  eDataType eDataType;
  void *pvData;
  void (*vCallback)(uint8_t);
  
}__attribute__((packed)) stDictionary;

typedef struct
{
  uint8_t uMessageId;
  uint8_t uReadWrite;
  eDataType eDataType;

}__attribute__((packed)) stMessageHeader;;



typedef struct Queue
{
  struct Queue *xpHead;
  struct Queue *xpNextElement;
  void *vpQueueData;
  size_t xlength;

}stQueueList;


extern void vProcessReceivedDataTask(void *pvParameters);

extern stDictionary xDictionary[configDICTIONARYSIZE];
extern SemaphoreHandle_t xDictionaryCreateQueueSemaphore();
extern int8_t iDictionaryAddDataToQueue(void * , int );
extern int8_t iDictionaryGetNextQueueElement(void * , int*);
extern SemaphoreHandle_t xQueueSemaphore;


