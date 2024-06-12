/*
 * MCU.h
 *
 *  Created on: May 8, 2024
 *      Author: carlo
 */

#ifndef APPLICATION_DRIVER_MODEL_SRC_MCU_H_
#define APPLICATION_DRIVER_MODEL_SRC_MCU_H_
#include "stdint.h"
#include "clock.h"
#include "spi.h"


class MCU
{

public:
	MCU(MCU &other)=delete;
	void operator=(const MCU &)=delete;
	static MCU *get_mcu_instance();
	void Set_Clock(Clock &);
	void Register_Spi(Spi<uint8_t,uint32_t> &);
	void Register_Can(Can_Obj &);
	Clock *clock;
	Spi<uint8_t,uint32_t>*spi;
	Can_Obj *can;

protected:
	MCU();
	static MCU * instance_;
	virtual ~MCU ();

};

#endif /* APPLICATION_DRIVER_MODEL_SRC_MCU_H_ */
