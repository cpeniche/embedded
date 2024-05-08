/*
 * MCU.h
 *
 *  Created on: May 8, 2024
 *      Author: carlo
 */

#ifndef APPLICATION_DRIVER_MODEL_SRC_MCU_H_
#define APPLICATION_DRIVER_MODEL_SRC_MCU_H_
#include "clock.h"
#include "spi.h"

class MCU
{

public:
	MCU(MCU &other)=delete;
	void operator=(const MCU &)=delete;
	static MCU *get_instance();
	Clock *clock;
	Spi	*spi;

protected:
	MCU();
	static MCU * instance_;
	virtual ~MCU ();

};

#endif /* APPLICATION_DRIVER_MODEL_SRC_MCU_H_ */
