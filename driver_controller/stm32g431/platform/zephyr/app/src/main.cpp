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
#include "adcInputBuilder.h"
#include "main.h"

static void vMain(void);
#define STACK_SIZE 512
#define PRIORITY K_PRIO_PREEMPT(15)
#define adcVolts(x) (float)(((float)x / 32768) - 1) * float(3.3)
/* Thread definitions*/
K_THREAD_DEFINE(MainThread, STACK_SIZE, vMain, NULL, NULL, NULL,
								PRIORITY, 0, 0);
K_MUTEX_DEFINE(spiMutex);								;

adcInputBuilder adcReader;
float voltage[2] = {0.0};


/******************************************
 *   Main
 *******************************************/
void vMain(void)
{

	inputInterface *adcinput = adcReader.factoryMethod(nullptr, nullptr, 0, 0, nullptr);
	uint8_t adcNumber = 0;
	uint8_t adcRead[3] = {0};

	while (1)
	{
		adcNumber = 0;
		getSpiMutex();
		adcinput->readInput(&adcNumber, 3);		
		releaseSpiMutex();
		adcinput->getInput(adcRead);
		voltage[adcNumber] = adcVolts(float(adcRead[0]<<8 | adcRead[1]));
		

		adcNumber = 1;		
		getSpiMutex();
		adcinput->readInput(&adcNumber, 3);
		releaseSpiMutex();
		adcinput->getInput(adcRead);
		
		voltage[adcNumber] = adcVolts(float(adcRead[0] <<8 | adcRead[1]));

		k_sleep(K_MSEC(10));
	}
}
/**
 * @brief Get the Spi Mutex object
 * 
 */
void getSpiMutex()
{
	k_mutex_lock(&spiMutex,K_FOREVER);
}

/**
 * @brief release the Spi Mutex Object
 * 
 */
void releaseSpiMutex()
{
	k_mutex_unlock(&spiMutex);
}
