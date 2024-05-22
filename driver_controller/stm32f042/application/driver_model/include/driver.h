/*
 * peripheral.h
 *
 *  Created on: May 8, 2024
 *      Author: carlo
 */

#ifndef APPLICATION_DRIVER_MODEL_SRC_PERIPHERAL_H_
#define APPLICATION_DRIVER_MODEL_SRC_PERIPHERAL_H_

template <class basetype, class errortype>
class Driver
{
public:

	Driver(){
		init=nullptr;
	};

	virtual ~Driver(){};
  virtual void Init() = 0;
  virtual void Read(basetype *) = 0;
  virtual void Write(basetype *) = 0;
  virtual errortype GetError()=0;

private:

  void (*init)(void);

};


template<class tx_header>
class Can_Tx_Msg{

public:
  tx_header header;
  uint8_t tx_data[8];

};

template<class rx_header>
class Can_Rx_Msg{

public:
  rx_header header;
  uint8_t tx_data[8];

};

template <class txbase,class rxbase>
class Can_Driver
{
public:

  Can_Driver(){};
  virtual ~Can_Driver(){};
  virtual void Init() = 0;
  virtual void Read(rxbase *) = 0;
  virtual void Write(Can_Tx_Msg<txbase>*) = 0;
  virtual uint32_t GetError()=0;

};

#endif /* APPLICATION_DRIVER_MODEL_SRC_PERIPHERAL_H_ */
