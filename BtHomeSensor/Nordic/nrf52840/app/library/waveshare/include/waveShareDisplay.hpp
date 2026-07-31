
#include <stdint.h>
#include "busInterface.hpp"

enum class dispError{
  None = 0,
  BufferError = -1,
};

uint8_t InitCmd[]={0x4d,0x78};

template <typename typeErr>
class waveShareDisplay : public displayInterface<typeErr>{

public:
  waveShareDisplay(){};
  typeErr SetInterface(busInterface *) override;
  typeErr Init(void)        override;
  typeErr Update(void)      override;
  typeErr PowerOn(void)     override;
  typeErr PowerOff(void)    override;
  typeErr ClearScreen(void) override;
  typeErr SetBuffer(uint8_t *buffer);

private:
  busInterface *_interface = nullptr;
  uint8_t *_buffer = nullptr;
};

template <typename typeErr>
inline typeErr waveShareDisplay<typeErr>::SetInterface(busInterface *Interface)
{
  _interface = Interface;
  return typeErr();
}

template <typename typeErr>
inline typeErr waveShareDisplay<typeErr>::Init(void)
{
  
  if(_buffer == nullptr)
    return typeErr(dispError::BufferError);  
  
  return typeErr(_interface->Write(InitCmd,sizeof(InitCmd)));
  
}

template <typename typeErr>
inline typeErr waveShareDisplay<typeErr>::Update()
{
  return typeErr();
}

template <typename typeErr>
inline typeErr waveShareDisplay<typeErr>::PowerOn(void)
{
  return typeErr();
}

template <typename typeErr>
inline typeErr waveShareDisplay<typeErr>::PowerOff(void)
{
  return typeErr();
}

template <typename typeErr>
inline typeErr waveShareDisplay<typeErr>::ClearScreen(void)
{
  return typeErr();
}

template <typename typeErr>
inline typeErr waveShareDisplay<typeErr>::SetBuffer(uint8_t  *buffer)
{
  _buffer=buffer;
  return typeErr();
}
