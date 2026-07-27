/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include "bmp280.h"
#include "bluetooth.h"


int main(int argc, char *argv[])
{

	uint8_t code;
	bmp280 *pressure = new bmp280();

	//if(initBluetooth() < 0)
		//return -1;
	
	while (1)
	{
		k_sleep(K_SECONDS(1));
		pressure->ReadDeviceCode(&code);
		printk("BMP280 Device Code: %x\n", code);
	}
	return 0;
}