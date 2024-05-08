#include "clock.h"


//Clock::Clock()
//{
//	driver=nullptr;
//	init = nullptr;
//
//}

void
Clock::Set_Init (void (*func)(void))
{
	init=func;
}
