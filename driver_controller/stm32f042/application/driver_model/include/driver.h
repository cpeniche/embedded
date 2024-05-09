/*
 * peripheral.h
 *
 *  Created on: May 8, 2024
 *      Author: carlo
 */

#ifndef APPLICATION_DRIVER_MODEL_SRC_PERIPHERAL_H_
#define APPLICATION_DRIVER_MODEL_SRC_PERIPHERAL_H_

class Driver
{
public:

	Driver(){
		init=nullptr;
	};
	virtual ~Driver(){};
  virtual void Init() = 0;
  virtual void Read() = 0;
  virtual void Write() = 0;

private:

  void (*init)(void);

};

#endif /* APPLICATION_DRIVER_MODEL_SRC_PERIPHERAL_H_ */
