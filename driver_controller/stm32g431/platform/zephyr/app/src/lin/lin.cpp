/*
 * Copyright (c) 2025 Embeint Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/net_buf.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/uart.h>
#include "lin.h"

LOG_MODULE_REGISTER(LIN, LOG_LEVEL_INF);

LIN::LIN(struct device *dev, uint8_t *rxBuffer, size_t length)
{	
	setDevice(dev);	
	error = uart_rx_enable(this->dev, rxBuffer, length, 100);
	
}

int8_t LIN::Transmit(uint8_t *buffer, size_t size)
{
		error = uart_tx(this->dev, buffer, size, SYS_FOREVER_US);
		clearFlag(TXDONE);
		return error;
}

void LIN::setCallback(uart_callback_t func)
{
	uart_callback_set(this->dev, func, (void *)this->dev);
}

void LIN::setFlag(eFlags flag)
{
	flags |= 1<<flag;
}

void LIN::clearFlag(eFlags flag)
{
	flags &= ~(1<<flag);
}
