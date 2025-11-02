
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

#define CALL_MEMBER_FN(object, ptrToMember) ((object)->*(ptrToMember))

static void DriverModuleTask(void);
/* Thread definitions*/
K_THREAD_DEFINE(DriverModule, STACK_SIZE, DriverModuleTask, NULL, NULL, NULL,
                PRIORITY, 0, 0);

uint8_t rxBuffer[7] = {0};
int16_t error;

void linCallBack(const struct device *dev, struct uart_event *evt, void *user_data);

LIN linDriver((struct device *)DEVICE_DT_GET(DT_NODELABEL(usart1)),
              rxBuffer, sizeof(rxBuffer), 0x10);
Buttons cButtons;

Buttons::Buttons()
{
}

void Buttons::Task(void)
{
  SetDriver(&linDriver);
  linDriver.setCallback(linCallBack);
  uint8_t *rxBufferPtr;
  struct data *motorStructsPtr[4] = {&stWindow, &stMirror, &stLock, nullptr};
  struct data *motorData = nullptr;
  uint8_t idx, idy = 0;

  while (1)
  {

    if (Read(NULL, 2) == 0)
    {
      k_sleep(K_MSEC(20));
      /* wait for data */
      if (getDataReady())
      {
        rxBufferPtr = getData();
        /*Loop through all motor structure data*/
        idy = 0;
        while (motorStructsPtr[idy] != nullptr)
        {
          motorData = motorStructsPtr[idy];
          if (rxBufferPtr[motorData->u8BufferIndex] ^ motorData->u8PrevState)
          {
            idx = 0;
            while (motorData->stActions[idx].motor != nullptr)
            {
              if ((motorData->stActions[idx].mask &
                   rxBufferPtr[motorData->u8BufferIndex]) ==
                  motorData->stActions[idx].mask)
              {
                CALL_MEMBER_FN(motorData->motorClassPtr,
                               motorData->stActions[idx].motor)();
                break;
              }
              idx++;
            }
          }
          motorData->u8PrevState = rxBufferPtr[motorData->u8BufferIndex];
          idy++;
        }
      }
    }
  }
}

bool Buttons::getDataReady()
{
  if ((Driver->getFlags() & (1 << Driver->RXDONE)) != 0)
    return true;
  else
    return false;
}

int8_t Buttons::Read(uint8_t *Buffer, size_t length)
{
  // if(Driver->getFlags() & (1<<Driver->TXDONE))
  return Driver->Transmit(Buffer, length);
}

void DriverModuleTask()
{

  cButtons.Task();
}

void linCallBack(const struct device *dev, struct uart_event *evt, void *user_data)
{
  linDriver.callBack(dev, evt, user_data);
}