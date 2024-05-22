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




template<class handletype, class cantxbase,
         class canrxbase, class canfilterbase>
class Can: public Can_Driver<cantxbase,canrxbase>
{
public:

  void Init() override;
  void Read(canrxbase *) override;
  void Write(Can_Tx_Msg<cantxbase>*) override;
  uint32_t GetError() override;

private:
  handletype drv_handle;
  uint32_t error;
  Can_Tx_Msg<cantxbase> tx;
  canrxbase rx;
  canfilterbase filter;

};


#include "../src/can.cpp"
#endif /* APPLICATION_DRIVER_MODEL_SRC_CAN_H_ */
