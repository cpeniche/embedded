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
  Can_Tx_Msg(){
    header.StdId=0xA;
    header.ExtId=0x0;
    header.IDE=0x0;
    header.RTR=0x0;
    header.DLC=8;
    header.TransmitGlobalTime=DISABLE;
  };
  virtual ~Can_Tx_Msg(){};
  tx_header header;
  uint8_t data[8];
  tx_header *get_header() {return &header;};
  uint8_t  *get_data_ptr() {return data;};

};

template<class rx_header>
class Can_Rx_Msg{

public:
  rx_header header;
  uint8_t data[8];
  rx_header *get_header() {return &header;};
  uint8_t  *get_data_ptr() {return data;};

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

  virtual ~Can_Driver(){};
  virtual void Init() = 0;
  virtual void Read(Can_Rx_Msg<rxbase> &) = 0;
  virtual void Write(Can_Tx_Msg<txbase> &) = 0;
  virtual uint32_t GetError()=0;

private:
  Can_Driver(const Can_Driver&)=delete;
  const Can_Driver& operator=(const Can_Driver&)=delete;


protected:
  Can_Driver(){};

};

template<class handletype, class cantxbase,
         class canrxbase, class canfilterbase>
class Can: public Can_Driver<cantxbase,canrxbase>
{
public:

  Can(): error(0){};
  virtual ~Can(){};
  void Init() override;
  void Read(Can_Rx_Msg<canrxbase> &) override;
  void Write(Can_Tx_Msg<cantxbase> &)override;

  uint32_t GetError() override;

private:

  handletype drv_handle;
  uint32_t error;
  //Can_Tx_Msg<cantxbase> tx;
  //Can_Rx_Msg<canrxbase> rx;
  //Can_Filter_Msg<canfilterbase> filter;

};

class Can_Obj
{

public:

  Can_Obj(){};
  virtual ~Can_Obj(){};
  Can<CAN_HandleTypeDef,
      CAN_TxHeaderTypeDef,
      CAN_RxHeaderTypeDef,
      CAN_FilterTypeDef> driver;

};


#include "../src/can.cpp"
#endif /* APPLICATION_DRIVER_MODEL_SRC_CAN_H_ */
