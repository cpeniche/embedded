#ifndef _clock_h_
#define _clock_h_

#include "driver.h"

class Clock : public Driver{

public:

	//Clock();
	//virtual ~Clock(){};
	void Set_Init(void (*)(void));

  void Init(){init();}

private:
	void (*driver)(void);
	void (*init)(void);

};

#endif
