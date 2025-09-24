#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(motorCtrl, LOG_LEVEL_DBG);
#include <zephyr/kernel.h>
#include "tle94103.h"


#define PRIORITY K_PRIO_PREEMPT(15) // k_thread_priority_get(k_current_get())
#define STACK_SIZE 1024
#define SLEEP_PERIOD K_SECONDS(1)

/* Thread definitions*/
K_THREAD_DEFINE(MotorControl, STACK_SIZE, vMotorCtrl, NULL, NULL, NULL,
								PRIORITY, 0, 0);

void vMotorCtrl(void)
{



  
}