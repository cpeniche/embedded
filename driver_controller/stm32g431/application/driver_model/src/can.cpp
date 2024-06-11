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

static void MX_CAN_Init(stCanHandleType *);

template<class handletype, class cantxbase,class canrxbase, class canfilterbase>
void Can<handletype, cantxbase, canrxbase, canfilterbase>::Init ()
{

  stCanFilterType filter={0x0};

  filter.FilterActivation=0x1;

  vCanInit(&drv_handle);
  xCanConfigureFilter(&(this->drv_handle),&filter);
  xCanActivateNotification(&(this->drv_handle),CAN_IT_RX_FIFO0_MSG_PENDING|
                                                   CAN_IT_RX_FIFO0_FULL|
                                                   CAN_IT_RX_FIFO0_OVERRUN);
  vSetInterruptHandleHook(vCanInterruptHandlerHook);
  xCanStart(&drv_handle);

}

template<class handletype, class cantxbase,
         class canrxbase, class canfilterbase>
void Can<handletype, cantxbase, canrxbase, canfilterbase>::Read (Can_Rx_Msg<canrxbase> &msg)
{

  uint32_t status=0x0;

  this->error = xCanGetReceiveMessage(&(this->drv_handle),
                       CAN_RX_FIFO0,
                       msg.get_header(),
                       msg.get_data_ptr());

}


template<class handletype, class cantxbase,
         class canrxbase, class canfilterbase>
void Can<handletype, cantxbase, canrxbase, canfilterbase>::Write (Can_Tx_Msg<cantxbase> &msg)
{
  uint32_t mailbox;


  this->error = xCanTransmitMessage(&(this->drv_handle),
                       msg.get_header(),
                       msg.get_data_ptr(),
                       &mailbox);


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
        if ((rx_msg.header.StdId & 0x7FF) == (it->Module_Id & 0x7FF))
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


void vCanInterruptRoutine()
{

  MCU* mcu = MCU::get_mcu_instance();
  mcu->can->driver.vCallInterruptHandleHook();
}


void vCanMessagePendingCallBack(stCanHandleType *hcan)
{

  MCU* mcu = MCU::get_mcu_instance();
  Can_Rx_Msg<CAN_RxHeaderTypeDef> rx_msg;
  mcu->can->driver.Read(rx_msg);
  mcu->can->driver.QueueRxMessage(rx_msg);

}


static void MX_CAN_Init(stCanHandleType *hcan)
{

   hcan->Instance = CAN;
   hcan->Init.Prescaler = 32;
   hcan->Init.Mode = CAN_MODE_NORMAL;
   hcan->Init.SyncJumpWidth = CAN_SJW_1TQ;
   hcan->Init.TimeSeg1 = CAN_BS1_4TQ;
   hcan->Init.TimeSeg2 = CAN_BS2_1TQ;
   hcan->Init.TimeTriggeredMode = DISABLE;
   hcan->Init.AutoBusOff = DISABLE;
   hcan->Init.AutoWakeUp = DISABLE;
   hcan->Init.AutoRetransmission = DISABLE;
   hcan->Init.ReceiveFifoLocked = DISABLE;
   hcan->Init.TransmitFifoPriority = DISABLE;
   hcan->State = HAL_CAN_STATE_RESET;
  if (HAL_CAN_Init(hcan) != HAL_OK)
  {
    //Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}


#endif
