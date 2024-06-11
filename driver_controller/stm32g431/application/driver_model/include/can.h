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
#include "halIncludes.h"
/* Hal type redefinitions*/

#define stCanHandleType         CAN_HandleTypeDef 
#define stCanTransmitHeaderType CAN_TxHeaderTypeDef 
#define stCanReceiveHeadetType  CAN_RxHeaderTypeDef 
#define stCanFilterType         CAN_FilterTypeDef


/* Hal function redefinitions */
#define vCanInit                      MX_CAN_Init
#define xCanConfigureFilter           HAL_CAN_ConfigFilter          
#define xCanActivateNotification      HAL_CAN_ActivateNotification  
#define vCanInterruptHandlerHook      HAL_CAN_IRQHandler
#define xCanStart                     HAL_CAN_Start                 
#define xCanGetReceiveMessage         HAL_CAN_GetRxMessage          
#define xCanTransmitMessage           HAL_CAN_AddTxMessage
#define vCanMessagePendingCallBack    HAL_CAN_RxFifo0MsgPendingCallback
#define vCanInterruptRoutine          CEC_CAN_IRQHandler


#ifdef __cplusplus
extern "C" {
#endif
  void vCanInterruptRoutine();
  void vCanMessagePendingCallBack(stCanHandleType *hcan);
  void vCanTask(void);

#ifdef __cplusplus
}
#endif

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

  void vCanTask(void);


  

};

#endif /* APPLICATION_DRIVER_MODEL_SRC_CAN_H_ */
