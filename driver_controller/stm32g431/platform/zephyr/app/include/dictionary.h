#pragma once


#define configMAXARRAYSIZE 10
#define configDICTIONARYSIZE 5
#define DELIMITER 0xFF

#define READ 0
#define WRITE 1

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
  eDataType enDataType;
  void *pvData;
  void (*vCallback)(uint8_t);
  
}__attribute__((packed)) stDictionary;

typedef struct
{
  uint8_t uMessageId;
  uint8_t uReadWrite;
  eDataType enDataType;

}__attribute__((packed)) stMessageHeader;;

typedef struct Queue
{
  struct Queue *xpHead;
  struct Queue *xpNextElement;
  void *vpQueueData;
  size_t xlength;

}stQueueList;

class Dictionary{

public:
  Dictionary(){
    k_sem_init(&QueueSemaphore,0,1);
    

  }
  ~Dictionary()= default;
  int8_t AddDataToQueue(void * , int );
  int8_t NextQueueElement(void * , int*);
  int SemaphoreTake(void){return k_sem_take(&QueueSemaphore,K_MSEC(50));}
  void SemaphoreGive(void){k_sem_give(&QueueSemaphore);}
  int8_t GetNextQueueElement(void **ppvprvData, int *piprvLength);
  int8_t LookUpMessage(uint8_t uprvMessageId);
  void GetCastedData(void *vpprvElement, eDataType eprvDataType, void *vpprvDest);
  stQueueList* CreateNode(void);
  void Task(void);

private:
    
  struct k_sem QueueSemaphore;
  stQueueList *xpQueue=NULL;
  stQueueList *xpQueueIndex=NULL;
  stQueueList *xpProcessDataIndex=NULL;
  stDictionary *xpDictionaryEntry=NULL;
  stDictionary xDictionary[configDICTIONARYSIZE]=
  {
    {.uMessageId=0x44,.enDataType=eUNSIGNED16,.pvData=nullptr,.vCallback=nullptr},
    {.uMessageId=0x55,.enDataType=eUNSIGNED16,.pvData=nullptr,.vCallback=nullptr},
    {.uMessageId=DELIMITER}
  };

};


Dictionary *getDictionaryInstance(void);

