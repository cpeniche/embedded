/*
 * Spi.cpp
 *
 *  Created on: May 8, 2024
 *      Author: carlo
 */

#include "spi.h"

Spi::Spi ()
{
	driver=nullptr;

}

Spi::~Spi ()
{
	// TODO Auto-generated destructor stub
}

void
Spi::Set_Driver (void(*func) (void))
{
	driver=func;
}
