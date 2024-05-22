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
	this->clock=nullptr;
	this->spi=nullptr;
	this->can=nullptr;

}


void MCU::Set_Clock (Clock &clock)
{
	this->clock=&clock;
}

void
MCU::Register_Can (Can<CAN_HandleTypeDef,
                   Can_Tx_Msg<CAN_TxHeaderTypeDef>,
                   CAN_RxHeaderTypeDef,
                   CAN_FilterTypeDef> &can_drv)
{
  this->can= &can_drv;
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

void MCU::Register_Spi (Spi<uint8_t, uint32_t> &spi_drv)
{
	this->spi= &spi_drv;
}

