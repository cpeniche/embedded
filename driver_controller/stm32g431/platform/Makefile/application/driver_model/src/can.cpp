/*
 * Can.cpp
 *
 *  Created on: May 22, 2024
 *      Author: carlo
 */

#ifndef _can_cpp_
#define _can_cpp_

#include <cstdint>
#include "can.h"
#include "mcu.h"
#include "dictionary.h"

static void MX_FDCAN_Init(stCanHandleType *);

template<class handletype, class cantxbase,class canrxbase, class canfilterbase>
void Can<handletype, cantxbase, canrxbase, canfilterbase>::Init ()
{

  stCanFilterType prvxFilter={0x0};

  prvxFilter.FilterIndex=0x1;
  prvxFilter.IdType=FDCAN_STANDARD_ID;
  prvxFilter.FilterType=FDCAN_FILTER_MASK;
  prvxFilter.FilterConfig=FDCAN_FILTER_TO_RXFIFO0;
  prvxFilter.FilterID1 = 0x0;

  vCanInit(&drv_handle);
  xCanConfigureFilter(&(this->drv_handle),&prvxFilter);
  xCanActivateNotification(&(this->drv_handle),FDCAN_IT_RX_FIFO0_NEW_MESSAGE|
                                                   FDCAN_IT_RX_FIFO0_FULL|
                                                   FDCAN_IT_RX_FIFO0_MESSAGE_LOST,0);
  vSetInterruptHandleHook(vCanInterruptHandlerHook);
  xCanStart(&drv_handle);

}

template<class handletype, class cantxbase,
         class canrxbase, class canfilterbase>
void Can<handletype, cantxbase, canrxbase, canfilterbase>::Read (Can_Rx_Msg<canrxbase> &msg)
{

  uint32_t status=0x0;

  this->error = xCanGetReceiveMessage(&(this->drv_handle),
                       FDCAN_RX_FIFO0,
                       msg.get_header(),
                       msg.get_data_ptr());

}


template<class handletype, class cantxbase,
         class canrxbase, class canfilterbase>
void Can<handletype, cantxbase, canrxbase, canfilterbase>::Write (Can_Tx_Msg<cantxbase> &msg)
{
  
  this->error = xCanTransmitMessage(&(this->drv_handle),
                       msg.get_header(),
                       msg.get_data_ptr());


}


template<class handletype, class cantxbase,
         class canrxbase, class canfilterbase>
uint32_t Can<handletype, cantxbase, canrxbase, canfilterbase>::GetError()
{
  return error;
}


template<class handletype, class cantxbase,
         class canrxbase, class canfilterbase>
uint32_t Can<handletype, cantxbase, canrxbase, canfilterbase>::QueueRxMessage(Can_Rx_Msg<canrxbase>& msg)
{
  if(MAX_RX_QUEUE_SIZE > rx_queue.size())
  {
    rx_queue.push_back(msg);
    return 0;
  }
  else
    return -1;
}

void Can_Obj::vCanTask(void)
{
  //static MCU* mcu = MCU::get_mcu_instance();
  Can_Rx_Msg<stCanReceiveHeadetType> rx_msg;

  while(1)
  {
    
    while(this->driver.Get_RxQueue_Size() != 0)
    {
      rx_msg=this->driver.Get_Queue_Msg();
      for(std::list<Dictionary>:: iterator it=dictionary.begin();
          it != dictionary.end(); it++)
      {
        if ((rx_msg.header.Identifier & 0x7FF) == (it->Module_Id & 0x7FF))
        {
            
            switch(it->dataType)
            {
              case UINT8:            
              *((uint8_t *)it->data_ptr)=rx_msg.data[0];
              break;

              case UINT16:
              *((uint16_t *)it->data_ptr)=*((uint8_t *)(rx_msg.data));
              break;
            
              default:
                break;
            }          
            it->Callback(WRITE);
        }
      }
    }
  }
}

/************* C Interface Function ***********/
void vCanTask(void)
{

  static MCU* mcu = MCU::get_mcu_instance();
  mcu->can->vCanTask();

}


void vCanInterruptRoutineZero()
{

  MCU* mcu = MCU::get_mcu_instance();
  mcu->can->driver.vCallInterruptHandleHook();
}

void vCanInterruptRoutineOne()
{

  MCU* mcu = MCU::get_mcu_instance();
  mcu->can->driver.vCallInterruptHandleHook();
}


void vCanGetReceiveFifoZeroCallBack(stCanHandleType *hcan, uint32_t ulprvArgs)
{

  MCU* mcu = MCU::get_mcu_instance();
  Can_Rx_Msg<stCanReceiveHeadetType> rx_msg;
  mcu->can->driver.Read(rx_msg);
  mcu->can->driver.QueueRxMessage(rx_msg);

}


static void MX_FDCAN_Init(stCanHandleType *hfdcan1)
{

  hfdcan1->Instance = FDCAN1;
  hfdcan1->Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1->Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1->Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1->Init.AutoRetransmission = DISABLE;
  hfdcan1->Init.TransmitPause = DISABLE;
  hfdcan1->Init.ProtocolException = DISABLE;
  hfdcan1->Init.NominalPrescaler = 32;
  hfdcan1->Init.NominalSyncJumpWidth = 1;
  hfdcan1->Init.NominalTimeSeg1 = 4;
  hfdcan1->Init.NominalTimeSeg2 = 5;
  hfdcan1->Init.DataPrescaler = 1;
  hfdcan1->Init.DataSyncJumpWidth = 1;
  hfdcan1->Init.DataTimeSeg1 = 1;
  hfdcan1->Init.DataTimeSeg2 = 1;
  hfdcan1->Init.StdFiltersNbr = 0;
  hfdcan1->Init.ExtFiltersNbr = 0;
  hfdcan1->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  
  if (HAL_FDCAN_Init(hfdcan1) != HAL_OK)
  {
    //Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */
}


#endif
