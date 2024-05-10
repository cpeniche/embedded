/*
 * peripheral.h
 *
 *  Created on: May 8, 2024
 *      Author: carlo
 */

#ifndef APPLICATION_DRIVER_MODEL_SRC_PERIPHERAL_H_
#define APPLICATION_DRIVER_MODEL_SRC_PERIPHERAL_H_

template <class basetype, class errortype>
class Driver
{
public:

	Driver(){
		init=nullptr;
	};

	virtual ~Driver(){};
  virtual void Init() = 0;
  virtual void Read(basetype *) = 0;
  virtual void Write(basetype *) = 0;
  virtual errortype GetError()=0;

private:

  void (*init)(void);

};

#endif /* APPLICATION_DRIVER_MODEL_SRC_PERIPHERAL_H_ */
