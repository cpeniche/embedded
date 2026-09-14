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
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/input/input.h>
LOG_MODULE_REGISTER(main_app, LOG_LEVEL_ERR);
#include "motorBuilder.h"
#include "bluetooth.h"

static constexpr struct gpio_dt_spec sw0 =
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(btn0), gpios, {0}); 
static constexpr struct gpio_dt_spec sw1 =
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(btn1), gpios, {0}); 

drv8838MotorBuilder drv8838MotorBuilder;
motorInterface *WaterValve = drv8838MotorBuilder.factoryMethod();

#define ENABLE_BLE


static void switch_callback_handler(struct input_event *evt,
				    void *user_data)
{
	
	if(evt->sync == 0)
	{
		return;
	}
	
	if(evt->code == INPUT_KEY_0 && evt->value == 1)
	{
		//LOG_INF("SW0 pressed");
		WaterValve->Up(0x0);
	}
	if(evt->code == INPUT_KEY_1 && evt->value == 1)
	{
		//LOG_INF("SW1 pressed");
		WaterValve->Down(0x0);
	}
}


int main(int argc, char *argv[])
{

	INPUT_CALLBACK_DEFINE(NULL, switch_callback_handler, NULL);

	k_sleep(K_FOREVER);

	return 0;
}


