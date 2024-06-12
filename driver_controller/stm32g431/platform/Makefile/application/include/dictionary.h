#ifndef _dictionary_h_
#define _dictionary_h_

#include <stdint.h>
#include <list>

enum data_type
{
  UINT8=0,
  UINT16,
  UINT32,
  UINT64,
  FLOAT
};

enum mode_type
{
  READ,
  WRITE
};

class Dictionary
{
 
public:
  Dictionary(uint16_t ID, data_type TYPE, void *data , void (*FUNC)(mode_type mode)):
            Module_Id{ID},dataType{TYPE},data_ptr{data},Callback{FUNC}{}
  uint16_t Module_Id;
  data_type dataType;
  void *data_ptr = nullptr;
  void (*Callback)(mode_type mode);

};

extern std::list<Dictionary> dictionary;

#endif