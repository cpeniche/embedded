/*
 * Copyright (c) 2018 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/uart.h>
#include "spiBuilder.h"
#include "zephyrSpi.h"
#include "adcInputBuilder.h"

static void vMain(void);
#define STACK_SIZE 512
#define PRIORITY K_PRIO_PREEMPT(15)
#define ACVoltage(x) (float)(((float)x / 32768) - 1) * 3.3
/* Thread definitions*/
K_THREAD_DEFINE(MainThread, STACK_SIZE, vMain, NULL, NULL, NULL,
								PRIORITY, 0, 0);

adcInputBuilder adcReader;
uint16_t temp;
uint8_t adcDataRead[3] = {0};
float voltage;

/******************************************
 *   Main
 *******************************************/
void vMain(void)
{

	inputInterface *adcinput = adcReader.factoryMethod(nullptr, nullptr, 0, 0, nullptr);
	/*canConfig();
	 */
	while (1)
	{
		adcinput->readInput(adcDataRead, sizeof(adcDataRead));
    temp = (adcDataRead[0] << 8) + adcDataRead[1];
    voltage = ACVoltage(temp);    
    LOG_INF("VOLTAGE : %f", voltage);
		k_sleep(K_MSEC(10));
	}
}
