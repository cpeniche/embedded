#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(button, LOG_LEVEL_DBG);
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "spiInterface.h"
#include "buttons.h"

Buttons::Buttons(spiInterface<uint8_t, int16_t> *spi)
{

  this->spi=spi;
  gpio_pin_configure(gpioa,10,GPIO_OUTPUT);
  gpio_pin_configure(gpioa,0,GPIO_OUTPUT);
  setLoad(0x1);
  setCs(0x1);
}

void Buttons::Read(void)
{

  setLoad(0x0);
  setLoad(0x1);
  setCs(0x0);
  spi->Read(&rxBuffer, sizeof(rxBuffer));
  setCs(0x1);
  LOG_DBG("Input Panel %x", rxBuffer);

}

void Buttons::Task(void)
{

  uint8_t prevRxBuffer=0x0;
  
  while(1){
    Read();	
    if(rxBuffer != prevRxBuffer){}
      

  }
}
