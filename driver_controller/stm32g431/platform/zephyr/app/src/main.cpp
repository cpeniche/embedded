/*
 * Copyright (c) 2018 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include "canConfig.h"
#include "spiBuilder.h"

static void vMain(void);
#define STACK_SIZE 1024
#define PRIORITY K_PRIO_PREEMPT(15)

/* Thread definitions*/
K_THREAD_DEFINE(MainThread, STACK_SIZE, vMain, NULL, NULL, NULL,
								PRIORITY, 0, 0);

/******************************************
 *   Main
 *******************************************/
void vMain(void)
{

	uint8_t testWrite = 0xAA;


	canConfig();
	SpiBuilder<uint8_t, int16_t> *spibuilder = new zephyrSpiBuilder<uint8_t, int16_t>();
	spiInterface<uint8_t, int16_t> *spi = spibuilder->factoryMethod(); 
	spi->Write(&testWrite,1);
	if (spi->GetError() != 0)
		LOG_ERR("Cannot send spi message %d", spi->GetError());
	while (1)
	{
		k_yield();
	}
}
