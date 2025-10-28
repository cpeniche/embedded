#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(motorCtrl, LOG_LEVEL_DBG);
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include "buttons.h"
#include "tle94103.h"


#define PRIORITY K_PRIO_PREEMPT(15) // k_thread_priority_get(k_current_get())
#define STACK_SIZE 1024
#define SLEEP_PERIOD K_SECONDS(1)

/* Thread definitions*/
K_THREAD_DEFINE(MotorControl, STACK_SIZE, vMotorCtrl, NULL, NULL, NULL,
								PRIORITY, 0, 0);

void vMotorCtrl(void)
{
	struct k_queue *buttonQueue = getQueue();
	uint32_t *data = NULL;

while (1)
	{
		data = (uint32_t*)k_queue_get(buttonQueue, K_FOREVER);
		k_sleep(K_MSEC(10));
	}
}