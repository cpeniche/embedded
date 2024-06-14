/*
 * Can.h
 *
 *  Created on: May 22, 2024
 *      Author: carlo
 */

#ifndef APPLICATION_DRIVER_MODEL_SRC_CAN_H_
#define APPLICATION_DRIVER_MODEL_SRC_CAN_H_

#include <list>
#include "driver.h"
#include "dictionary.h"
#include "halIncludes.h"
/* Hal type redefinitions*/

#define stCanHandleType         FDCAN_HandleTypeDef 
#define stCanTransmitHeaderType FDCAN_TxHeaderTypeDef 
#define stCanReceiveHeadetType  FDCAN_RxHeaderTypeDef 
#define stCanFilterType         FDCAN_FilterTypeDef


/* Hal function redefinitions */
#define vCanInit                      MX_FDCAN_Init
#define xCanConfigureFilter           HAL_FDCAN_ConfigFilter          
#define xCanActivateNotification      HAL_FDCAN_ActivateNotification  
#define vCanInterruptHandlerHook      HAL_FDCAN_IRQHandler
#define xCanStart                     HAL_FDCAN_Start                 
#define xCanGetReceiveMessage         HAL_FDCAN_GetRxMessage          
#define xCanTransmitMessage           HAL_FDCAN_AddMessageToTxFifoQ
#define vCanGetReceiveFifoZeroCallBack    HAL_FDCAN_RxFifo0Callback
#define vCanInterruptRoutineZero      FDCAN1_IT0_IRQHandler
#define vCanInterruptRoutineOne       FDCAN1_IT1_IRQHandler


template<class tx_header>
class Can_Tx_Msg{

public:
  Can_Tx_Msg(){
    header.Identifier = 0x55;
    header.IdType = FDCAN_STANDARD_ID;
    header.TxFrameType = FDCAN_DATA_FRAME;
    header.DataLength = FDCAN_DLC_BYTES_3;
    header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    header.BitRateSwitch = FDCAN_BRS_OFF;
    header.FDFormat = FDCAN_CLASSIC_CAN;
    header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    header.MessageMarker = 0;
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

  static constexpr uint32_t MAX_RX_QUEUE_SIZE=10;
public:

  Can(): error(0){};
  virtual ~Can(){};
  void Init() override;
  void Read(Can_Rx_Msg<canrxbase> &) override;
  void Write(Can_Tx_Msg<cantxbase> &)override;

  void vSetInterruptHandleHook(void (*vFunc)(handletype* )){prvInterruptHandleHooK=vFunc;};
  void vCallInterruptHandleHook(){prvInterruptHandleHooK(&drv_handle);};
  uint32_t QueueRxMessage(Can_Rx_Msg<canrxbase>&);
  uint32_t GetError() override;
  uint32_t Get_RxQueue_Size(){return rx_queue.size();}
  Can_Rx_Msg<canrxbase> Get_Queue_Msg(){
  Can_Rx_Msg<canrxbase> msg=rx_queue.front();
  rx_queue.pop_front();
    return msg;
  }
  
  //friend void vCanInterruptRoutine();
  //friend void vCanMessagePendingCallBack(stCanHandleType *hcan);

private:

  std::list<Can_Rx_Msg<canrxbase>>rx_queue;
  void (*prvInterruptHandleHooK)(handletype *);
  handletype drv_handle;
  uint32_t error;

};

class Can_Obj
{

typedef void (*pvfunction)(void);

public:

  Can_Obj(){};
  virtual ~Can_Obj(){};
  Can<stCanHandleType,
      stCanTransmitHeaderType,
      stCanReceiveHeadetType,
      stCanFilterType> driver;

  void vCanTask(void)
  {
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
};

#endif /* APPLICATION_DRIVER_MODEL_SRC_CAN_H_ */
