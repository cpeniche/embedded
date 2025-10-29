#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(button, LOG_LEVEL_DBG);
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include "lin.h"
#include "buttons.h"


static void ReadButtonsTask(void);
#define STACK_SIZE 1024
#define PRIORITY K_PRIO_PREEMPT(15)

/* Thread definitions*/
K_THREAD_DEFINE(ButtonsThread, STACK_SIZE, ReadButtonsTask, NULL, NULL, NULL,
								PRIORITY, 0, 0);

#define ProtectedIdentifier 0x10
#define SynchData 0x55          
static uint8_t IdentifierFieldParity(uint8_t);
uint8_t rxBuffer[7]={0};
uint8_t txBuffer[2]={0};
int16_t error;

K_QUEUE_DEFINE(buttonsQueue);

static void LINRxCallback(const struct device *dev, struct uart_event *evt, void *user_data);
LIN linDriver((struct device *)DEVICE_DT_GET(DT_NODELABEL(usart1)),
                           rxBuffer, sizeof(rxBuffer)); 
Buttons cButtons;

int8_t Buttons::Read(uint8_t *Buffer, size_t length)
{  
  Driver->Transmit(Buffer, length);
  return Driver->getError();
}

void ReadButtonsTask(void )
{
  uint8_t prevRxBuffer[4]={0x0};
  linDriver.setCallback(LINRxCallback);
  cButtons.SetDriver(&linDriver);
  uint8_t rxCounter =0;

  txBuffer[0] = SynchData;
  txBuffer[1] = ProtectedIdentifier << 2 | IdentifierFieldParity(ProtectedIdentifier);

  while(1)
  {
    if((error= cButtons.Read(txBuffer,sizeof(txBuffer))) != 0)
    {
      LOG_DBG("Lin Driver Error (%d)",error);
    }
    rxCounter=0;
    do{
      k_sleep(K_MSEC(5));
      if(rxCounter > 10)
      {
        linDriver.setFlag(LIN::RXERROR);
        break;
      }
      rxCounter++;

    }while((linDriver.getFlags() & (1<<LIN::RXDONE))==0); 
    if (!(linDriver.getFlags() & (1<<LIN::RXERROR)))
      memcpy(prevRxBuffer,&rxBuffer[2],4);
    if(*((uint32_t *)prevRxBuffer) != *((uint32_t *)&rxBuffer[2]))
      k_queue_append(&buttonsQueue, ((uint32_t *)&rxBuffer[2]));
    k_sleep(K_MSEC(20));
  }
}

uint8_t IdentifierFieldParity(uint8_t u8prvData)
{
	uint8_t u8p0, u8p1;

	u8p0 = (u8prvData ^ (u8prvData >> 1) ^ (u8prvData >> 2) ^ (u8prvData >> 4)) & 0x1;
	u8p1 = (~((u8prvData >> 1) ^ (u8prvData >> 3) ^ (u8prvData >> 4) ^ (u8prvData >> 5))) & 0x1;

	return (u8p0 << 1 | u8p1);
}

static void LINRxCallback(const struct device *dev, struct uart_event *evt, void *user_data)
{
	
  int8_t rc;
	LOG_DBG("EVENT: %d", evt->type);

	switch (evt->type)
	{
	case UART_TX_DONE:
		linDriver.setFlag(LIN::TXDONE);
    linDriver.clearFlag(LIN::RXDONE);
		break;
  
	case UART_RX_BUF_REQUEST:
	case UART_RX_BUF_RELEASED:
	case UART_RX_DISABLED:
		break;
	case UART_RX_RDY:
    rc = uart_rx_buf_rsp(linDriver.getDevice(), rxBuffer,sizeof(rxBuffer));
		linDriver.setFlag(LIN::RXDONE);
    linDriver.clearFlag(LIN::RXERROR);
		break;    
	default:
		LOG_WRN("Unhandled event %d", evt->type);
	}
}

struct k_queue *getQueue(void)
{
  return &buttonsQueue;
}

