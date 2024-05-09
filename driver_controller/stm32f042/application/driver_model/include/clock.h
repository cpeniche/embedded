#ifndef _clock_h_
#define _clock_h_

#include "driver.h"

class Clock{

public:

	Clock(){};
	virtual ~Clock(){};
  virtual void Init() const =0;

};


class HSI_Clock: public Clock{

public:
	void Init() const override;

};

#endif
