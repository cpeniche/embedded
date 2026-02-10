
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(button, LOG_LEVEL_DBG);
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/pm.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/poweroff.h>
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

void ExecutePowerDown(struct k_timer *timer);
K_TIMER_DEFINE(powerDown, ExecutePowerDown, nullptr);

static void DriverModuleTask(void);
/* Thread definitions*/
K_THREAD_DEFINE(DriverModule, STACK_SIZE, DriverModuleTask, NULL, NULL, NULL,
                PRIORITY, 0, 0);

int16_t error;
uint8_t rxBuffer[2][RXMSGLENGTH] = {0};
uint8_t currMsg[RXMSGLENGTH - 1] = {0};
void linCallBack(const struct device *dev, struct uart_event *evt, void *user_data);

Buttons cButtons;
/**
 * @brief Buttons task declaration
 *
 */

void Buttons::Task(void)
{

  struct data *motorStructsPtr[4] = {&stWindow, &stMirror, &stLock, nullptr};
  struct data *motorData = nullptr;
  uint8_t idx, idy = 0;
  inputInterface *input = linButtonsReader.factoryMethod(device, rxBuffer[0], RXMSGLENGTH, 0x10, linCallBack);

  while (1)
  {

    if (input->readInput(nullptr, 2) == 0)
    {

      k_msleep(20);
      /* wait for data */
      if (input->isDataReady() && input->getInput(currMsg) == 0)
      {

        k_timer_start(&powerDown, K_MSEC(5000), K_NO_WAIT);

        if (CalculateChecksum(currMsg, 5) != currMsg[5])
        {
          LOG_ERR("LIN Message Checksum Error");
          continue;
        }
        else
        {
          if (currMsg[2] == maskMIRRORSELECTRIGHT)
            *stMirror.stActions[0x0].side = RIGHT;
          if (currMsg[2] == maskMIRRORSELECTLEFT)
            *stMirror.stActions[0x0].side = LEFT;

          /*Loop through all motor structure data*/
          idy = 0;
          while (motorStructsPtr[idy] != nullptr)
          {
            motorData = motorStructsPtr[idy];
            if (currMsg[motorData->u8BufferIndex] ^ motorData->u8PrevState)
            {
              idx = 0;
              while (motorData->stActions[idx].motor != nullptr)
              {
                if ((motorData->stActions[idx].mask &
                     currMsg[motorData->u8BufferIndex]) ==
                    motorData->stActions[idx].mask)
                {
                  CALL_MEMBER_FN(motorData->motorClassPtr,
                                 motorData->stActions[idx].motor)(*motorData->stActions[idx].side);
                  break;
                }
                idx++;
              }
            }
            motorData->u8PrevState = currMsg[motorData->u8BufferIndex];
            idy++;
          }
        }
      }
    }
  }
}

/**
 * @brief Lin frame checksum calculations
 *
 * @param ptr       : buffer pointer
 * @param length    : buffer length
 * @return uint8_t  : checksum value
 */
uint8_t Buttons::CalculateChecksum(uint8_t *ptr, size_t length)
{
  uint16_t chkSum = 0;

  for (uint8_t idx = 0; idx < length; idx++)
    chkSum += ptr[idx];
  return (~(uint8_t)chkSum) - 1;
}

/**
 * @brief Declare Driver task
 *
 */
void DriverModuleTask()
{

  cButtons.Task();
}

/**
 * @brief Callback routine called when a lin message is received
 *
 * @param dev         : dts lin driver module
 * @param evt         : event received
 * @param user_data   : user data passed to the callback routine
 */
void linCallBack(const struct device *dev, struct uart_event *evt, void *user_data)
{
  cButtons.getDriver()->callBack(dev, evt, user_data);
}

/**
 * @brief Power down execution function
 *
 * @param timer
 */
void ExecutePowerDown(struct k_timer *timer)
{

  if (pm_device_wakeup_is_capable(device) &&
      pm_device_wakeup_enable(device, true) &&
      pm_device_wakeup_is_enabled(device))
  {
    LOG_INF("Entering low power mode");
    pm_state_set(PM_STATE_SUSPEND_TO_IDLE, 1);
  }
}