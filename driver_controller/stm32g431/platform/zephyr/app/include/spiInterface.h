/*
 * peripheral.h
 *
 *  Created on: May 8, 2024
 *      Author: carlo
 */
#ifndef SPI_INTERFACE_H_
#define SPI_INTERFACE_H_

template <class basetype, class errortype>
class spiInterface
{
public:
  // spiInterface()
  // {
  //   init = nullptr;
  // };

  virtual ~spiInterface() {};
  virtual void Init() = 0;
  virtual void Read(basetype *) = 0;
  virtual void Write(basetype *) = 0;
  virtual errortype GetError() = 0;

/* private:
  void (*init)(void); */
};

#endif