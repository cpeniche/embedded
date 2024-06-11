/*
 * MCU.cpp
 *
 *  Created on: May 8, 2024
 *      Author: carlo
 */

#include "can.h"
#include "mcu.h"

MCU *MCU::instance_ = nullptr;

MCU::MCU ()
{
	this->clock=nullptr;
	this->spi=nullptr;
	this->can=nullptr;

}

MCU *MCU::get_mcu_instance()
{
  if(instance_ == nullptr)
  {
    instance_=new MCU();
  }
  return instance_;
}

void MCU::Set_Clock (Clock &clock)
{
	this->clock=&clock;
}


void
MCU::Register_Can (Can_Obj &can)
{
  this->can= &can;
}

MCU::~MCU ()
{

}

void MCU::Register_Spi (Spi<uint8_t, uint32_t> &spi_drv)
{
	this->spi= &spi_drv;
}

