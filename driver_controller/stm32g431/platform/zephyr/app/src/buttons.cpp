#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(button, LOG_LEVEL_DBG);
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/spi.h>
#include "spiBuilder.h"
#include "zephyrSpi.h"
#include "motorInterface.h"
#include "motorBuilder.h"
#include "tle94103.h"
#include "drv8838.h"
#include "vnh5019a.h"
#include "lin.h"
#include "buttons.h"

#define STACK_SIZE 1024
#define PRIORITY K_PRIO_PREEMPT(15)

static void DriverModuleTask(void );
/* Thread definitions*/
K_THREAD_DEFINE(DriverModule, STACK_SIZE, DriverModuleTask, NULL, NULL, NULL,
								PRIORITY, 0, 0);
         
uint8_t rxBuffer[7]={0};
uint8_t txBuffer[2]={0};
int16_t error;

K_QUEUE_DEFINE(buttonsQueue);

void linCallBack(const struct device *dev, struct uart_event *evt, void *user_data);

LIN linDriver((struct device *)DEVICE_DT_GET(DT_NODELABEL(usart1)),
                           rxBuffer, sizeof(rxBuffer), 0x10); 
Buttons cButtons;

struct k_queue *getQueue(void)
{
  return &buttonsQueue;
}

void Buttons::Task(void)
{
  uint32_t prevRxBuffer[4]={0};
  SetDriver(&linDriver);
  linDriver.setCallback(linCallBack);
  uint8_t *rxBufferPtr;
  bool queueData=false;
  
  while(1)
  {
    
    if(Read(NULL,2)== 0)
    {      
      k_sleep(K_MSEC(20));
      /* wait for data */
      if(getDataReady())
      {         
        rxBufferPtr=getData(); 
        for(int idx=0; idx<4; idx++){
          if(prevRxBuffer[idx] != rxBufferPtr[idx]) 
          {                   
            queueData=true;
          }
          prevRxBuffer[idx]=rxBufferPtr[idx];
        }
        if(queueData)
        {                   
          mirror->Down();
          latch->Down();
          window->Down();
          k_sleep(K_MSEC(1));
          latch->Idle();
          window->Idle();
          queueData=false;
        }
      }     
    }         
  }
}

bool Buttons::getDataReady()
{
  if((Driver->getFlags() & (1<<Driver->RXDONE)) != 0)
    return true;
  else
    return false;
}

int8_t Buttons::Read(uint8_t *Buffer, size_t length)
{  
  //if(Driver->getFlags() & (1<<Driver->TXDONE))
    return Driver->Transmit(Buffer, length);
}


void DriverModuleTask()
{
  cButtons.Task();
}

void linCallBack(const struct device *dev, struct uart_event *evt, void *user_data)
{
  linDriver.callBack(dev,evt,user_data);
}