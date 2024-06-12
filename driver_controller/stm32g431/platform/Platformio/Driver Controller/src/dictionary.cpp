#include "dictionary.h"
#include "motor.h"


std::list<Dictionary> dictionary
{
  Dictionary(0x55,UINT8,&window_dir,Motor_Callback),
  Dictionary(0x56,UINT16,nullptr,nullptr),
  Dictionary(0x57,FLOAT,nullptr,nullptr),
  Dictionary(0x58,UINT8,nullptr,nullptr),
  Dictionary(0x59,UINT8,nullptr,nullptr),
};