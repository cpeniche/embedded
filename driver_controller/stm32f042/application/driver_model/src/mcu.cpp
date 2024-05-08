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
	clock = new Clock();
	spi = new Spi();
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


