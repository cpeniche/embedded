/*
 * Spi.h
 *
 *  Created on: May 8, 2024
 *      Author: carlo
 */

#ifndef APPLICATION_DRIVER_MODEL_SRC_SPI_H_
#define APPLICATION_DRIVER_MODEL_SRC_SPI_H_

class Spi
{
public:
	Spi ();
	virtual
	~Spi ();
	void Set_Driver(void (*)(void));


protected:
	void (*driver)(void);
	void (*Init)(void);

};

#endif /* APPLICATION_DRIVER_MODEL_SRC_SPI_H_ */
