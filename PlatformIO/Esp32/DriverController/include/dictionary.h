#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

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
  } eDataTYPE;

  typedef struct
  {
    uint8_t uMessageId;
    uint8_t uReadWrite;
    eDataTYPE eDataType;
    void (*vCallback)(uint8_t, eDataTYPE, void *);

  } __attribute__((packed)) stDictionary;

  typedef struct
  {
    uint8_t uMessageId;
    uint8_t uReadWrite;
    eDataTYPE eDataType;

  } __attribute__((packed)) stMessageHeader;
  ;

  typedef struct Queue
  {
    struct Queue *xpHead;
    struct Queue *xpNextElement;
    void *vpQueueData;
    size_t xlength;

  } stQueueList;

  extern void vProcessReceivedDataTask(void *pvParameters);
  extern void vGetCastedData(void *vpprvData, eDataTYPE eprvDataType, void *vpprvDest);

  extern stDictionary xDictionary[configDICTIONARYSIZE];
  extern SemaphoreHandle_t xDictionaryCreateQueueSemaphore();
  extern int8_t iDictionaryAddDataToQueue(void *, int);
  extern int8_t iDictionaryGetNextQueueElement(void *, int *);
  extern SemaphoreHandle_t xQueueSemaphore;

#ifdef __cplusplus
}
#endif
