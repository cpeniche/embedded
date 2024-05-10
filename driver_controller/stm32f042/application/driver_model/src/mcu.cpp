/*
 * MCU.cpp
 *
 *  Created on: May 8, 2024
 *      Author: carlo
 */

#include "mcu.h"

MCU *MCU::instance_ = nullptr;

MCU::MCU ()
{
//	this->clock=nullptr;
//	this->spi=nullptr;
}


void MCU::Set_Clock (Clock &clock)
{
	this->clock=&clock;
}

MCU::~MCU ()
{

}

MCU *MCU::get_instance()
{
	if(instance_ == nullptr)
	{
		instance_=new MCU();
	}
	return instance_;
}

void MCU::Register_Spi (Spi<uint8_t, HAL_StatusTypeDef> &spi_drv)
{
	this->spi=&spi_drv;
}

