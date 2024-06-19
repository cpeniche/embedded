#include "dictionary.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "motor.h"


std::list<Dictionary> dictionary
{
  Dictionary(0x55,UINT8,&eWindowDirection,vMotorCallback),
  Dictionary(0x56,UINT8,&eLatchPosition,vDoorLatchCallback),
  Dictionary(0x57,FLOAT,nullptr,nullptr),
  Dictionary(0x58,UINT8,nullptr,nullptr),
  Dictionary(0x59,UINT8,nullptr,nullptr),
};