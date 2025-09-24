/*
 * peripheral.h
 *
 *  Created on: May 8, 2024
 *      Author: carlo
 */
#pragma once

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
  virtual void Read(basetype *, size_t) = 0;
  virtual void Write(basetype *, size_t) = 0;
  virtual errortype GetError() = 0;

  /* private:
    void (*init)(void); */
};