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

static void MX_CAN_Init(CAN_HandleTypeDef *);

template<class handletype, class cantxbase,class canrxbase, class canfilterbase>
void Can<handletype, cantxbase, canrxbase, canfilterbase>::Init ()
{

  CAN_FilterTypeDef filter={0x0};

  filter.FilterActivation=0x1;

  MX_CAN_Init(&drv_handle);
  HAL_CAN_ConfigFilter(&(this->drv_handle),&filter);
  HAL_CAN_Start(&drv_handle);

}

template<class handletype, class cantxbase,
         class canrxbase, class canfilterbase>
void Can<handletype, cantxbase, canrxbase, canfilterbase>::Read (Can_Rx_Msg<canrxbase> &msg)
{

  uint32_t status=0x0;

  this->error = HAL_CAN_GetRxMessage(&(this->drv_handle),
                       CAN_RX_FIFO0,
                       msg.get_header(),
                       msg.get_data_ptr());

}


template<class handletype, class cantxbase,
         class canrxbase, class canfilterbase>
void Can<handletype, cantxbase, canrxbase, canfilterbase>::Write (Can_Tx_Msg<cantxbase> &msg)
{
  uint32_t mailbox;


  this->error = HAL_CAN_AddTxMessage(&(this->drv_handle),
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


static void MX_CAN_Init(CAN_HandleTypeDef *hcan)
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
