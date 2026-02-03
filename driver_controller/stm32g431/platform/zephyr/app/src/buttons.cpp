
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(button, LOG_LEVEL_DBG);
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
//#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/spi.h>
#include "spiBuilder.h"
#include "zephyrSpi.h"
#include "can.h"
#include "motorInterface.h"
#include "motorBuilder.h"
#include "tle94103.h"
#include "drv8838.h"
#include "vnh5019a.h"
#include "lin.h"
#include "adcInputBuilder.h"
#include "linInputBuilder.h"
#include "buttons.h"

#define STACK_SIZE 1024
#define PRIORITY K_PRIO_PREEMPT(15)


#define CALL_MEMBER_FN(object, ptrToMember) ((object)->*(ptrToMember))

static void DriverModuleTask(void);
/* Thread definitions*/
K_THREAD_DEFINE(DriverModule, STACK_SIZE, DriverModuleTask, NULL, NULL, NULL,
                PRIORITY, 0, 0);

int16_t error;

void linCallBack(const struct device *dev, struct uart_event *evt, void *user_data);

Buttons cButtons;

void Buttons::Task(void)
{

  uint8_t *rxBufferPtr;
  struct data *motorStructsPtr[4] = {&stWindow, &stMirror, &stLock, nullptr};
  struct data *motorData = nullptr;
  uint8_t idx, idy = 0;
  inputInterface *input = linButtonsReader.factoryMethod(device, rxBuffer, sizeof(rxBuffer), 0x10, linCallBack);
  //inputInterface *adcinput = adcButtonsReader.factoryMethod(nullptr, nullptr, 0, 0, nullptr);

  while (1)
  {
    /*adcinput->readInput(adcDataRead, sizeof(adcDataRead));
    temp = (adcDataRead[0] << 8) + adcDataRead[1];
    voltage = ACVoltage(temp);    
    LOG_INF("VOLTAGE : %f", voltage);*/

    if (input->readInput(nullptr, 2) == 0)
    {

      /* wait for data */
      if (input->getDataReady())
      {
        rxBufferPtr = input->getInput();

        if (CalculateChecksum(rxBufferPtr, 5) != rxBufferPtr[5])
        {
          LOG_ERR("LIN Message Checksum Error");
          continue;
        }
        else
        {
          if (rxBufferPtr[2] == maskMIRRORSELECTRIGHT)
            *stMirror.stActions[0x0].side = RIGHT;
          if (rxBufferPtr[2] == maskMIRRORSELECTLEFT)
            *stMirror.stActions[0x0].side = LEFT;

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
                                 motorData->stActions[idx].motor)(*motorData->stActions[idx].side);
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
}

uint8_t Buttons::CalculateChecksum(uint8_t *ptr, size_t length)
{
  uint16_t chkSum = 0;

  for (uint8_t idx = 0; idx < length; idx++)
    chkSum += ptr[idx];
  return (~(uint8_t)chkSum) - 1;
}

void DriverModuleTask()
{

  cButtons.Task();
}

void linCallBack(const struct device *dev, struct uart_event *evt, void *user_data)
{
  cButtons.getDriver()->callBack(dev, evt, user_data);
}
