/*
 * Can.h
 *
 *  Created on: May 22, 2024
 *      Author: carlo
 */

#ifndef APPLICATION_DRIVER_MODEL_SRC_CAN_H_
#define APPLICATION_DRIVER_MODEL_SRC_CAN_H_

#include "driver.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_can.h"

template<class tx_header>
class Can_Tx_Msg{

public:
  tx_header header;
  uint8_t data[8];

};

template<class rx_header>
class Can_Rx_Msg{

public:
  rx_header header;
  uint8_t data[8];

};

template<class canfilterbase>
class Can_Filter_Msg{

public:
  canfilterbase filter;
};

template <class txbase,class rxbase>
class Can_Driver
{
public:

  Can_Driver(){};
  virtual ~Can_Driver(){};
  virtual void Init() = 0;
  virtual void Read(Can_Rx_Msg<rxbase> *) = 0;
  virtual void Write(Can_Tx_Msg<txbase>*) = 0;
  virtual uint32_t GetError()=0;

};

template<class handletype, class cantxbase,
         class canrxbase, class canfilterbase>
class Can: public Can_Driver<cantxbase,canrxbase>
{
public:

  void Init() override;
  void Read(Can_Rx_Msg<canrxbase> *) override;
  void Write(Can_Tx_Msg<cantxbase>*) override;
  uint32_t GetError() override;

private:
  handletype drv_handle;
  uint32_t error;
  Can_Tx_Msg<cantxbase> tx;
  Can_Rx_Msg<canrxbase> rx;
  Can_Filter_Msg<canfilterbase> filter;

};

class Can_Obj
{

public:

  Can_Obj(){};
  virtual ~Can_Obj(){};
  Can<CAN_HandleTypeDef,
  Can_Tx_Msg<CAN_TxHeaderTypeDef>,
  Can_Rx_Msg<CAN_RxHeaderTypeDef>,
  Can_Filter_Msg<CAN_FilterTypeDef> > driver;

};


#include "../src/can.cpp"
#endif /* APPLICATION_DRIVER_MODEL_SRC_CAN_H_ */
