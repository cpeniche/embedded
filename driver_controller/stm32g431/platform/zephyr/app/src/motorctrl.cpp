#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(motorCtrl, LOG_LEVEL_DBG);
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include "lin.h"
#include "buttons.h"
#include <zephyr/drivers/spi.h>
#include "spiBuilder.h"
#include "zephyrSpi.h"
#include "motorctrl.h"
#include "tle94103.h"

#define maskLOCK 0x01
#define maskUNLOCK 0x02
#define maskRIGHWINDOWUP 0x04
#define maskRIGHWINDOWDWON 0x08
#define maskLEFTWINDOWUP 0x10
#define maskLEFTWINDOWDWON 0x20
#define maskMIRRORUP 0x40
#define maskMIRRORDOWN 0x80

#define PRIORITY K_PRIO_PREEMPT(15) // k_thread_priority_get(k_current_get())
#define STACK_SIZE 1024
#define SLEEP_PERIOD K_SECONDS(1)

static void vMotorCtrl(void);
/* Thread definitions*/
K_THREAD_DEFINE(MotorControl, STACK_SIZE, vMotorCtrl, NULL, NULL, NULL,
								PRIORITY, 0, 0);

const struct device *gpioa = DEVICE_DT_GET(DT_NODELABEL(gpioa));
zephyrSpiBuilder<uint8_t, int16_t> spibuilder;
spiInterface<uint8_t, int16_t> *spi = spibuilder.factoryMethod();
void setCs(uint8_t value) { gpio_pin_set(gpioa, 10, value); }

static void vMotorCtrl(void)
{
	struct k_queue *buttonQueue = getQueue();
	uint32_t *data = NULL;
	Tle94103 tle94103Spi(spi);
	uint8_t txBuffer = 0x0;

	while (1)
	{
		data = (uint32_t *)k_queue_get(buttonQueue, K_FOREVER);
		tle94103Spi.WriteRegister(Tle94103::HBCTRL, &txBuffer);
		k_sleep(K_MSEC(10));
	}
}