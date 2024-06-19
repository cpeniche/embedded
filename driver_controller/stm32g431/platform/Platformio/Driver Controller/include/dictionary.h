#ifndef _dictionary_h_
#define _dictionary_h_

#include <stdint.h>
#include <list>

enum eDataType
{
  UINT8=0,
  UINT16,
  UINT32,
  UINT64,
  FLOAT
};

enum eReadWrite
{
  READ,
  WRITE
};

class Dictionary
{
 
public:
  Dictionary(uint16_t ID, eDataType TYPE, void *data , void (*FUNC)(eReadWrite mode)):
            Module_Id{ID},dataType{TYPE},data_ptr{data},Callback{FUNC}{}
  uint16_t Module_Id;
  eDataType dataType;
  void *data_ptr = nullptr;
  void (*Callback)(eReadWrite mode);

};

extern std::list<Dictionary> dictionary;

#endif