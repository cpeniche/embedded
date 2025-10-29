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
void callBack(const struct device *dev, struct uart_event *evt, void *user_data);

LIN::LIN(struct device *dev, uint8_t *rxBuffer, size_t length, uint8_t Identifier)
{	
	setDevice(dev);	
	setProtectedID(Identifier);
	error = uart_rx_enable(this->dev, rxBuffer, length, 100);

}

int8_t LIN::Transmit(uint8_t *buffer, size_t size)
{
		txBuffer[0] = SynchData;
		txBuffer[1] = (protectedId << 2) |  IdentifierFieldParity(protectedId);
		if(buffer != NULL)
			for(size_t idx=2; idx<size; idx++)
				txBuffer[idx] = buffer[idx-2];
		error = uart_tx(this->dev, txBuffer, size, SYS_FOREVER_US);
		if(error != 0)
			setFlag(RXERROR);
		else
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

uint8_t LIN::IdentifierFieldParity(uint8_t u8prvData)
{
	uint8_t u8p0, u8p1;

	u8p0 = (u8prvData ^ (u8prvData >> 1) ^ (u8prvData >> 2) ^ (u8prvData >> 4)) & 0x1;
	u8p1 = (~((u8prvData >> 1) ^ (u8prvData >> 3) ^ (u8prvData >> 4) ^ (u8prvData >> 5))) & 0x1;

	return (u8p0 << 1 | u8p1);
}

void LIN::callBack(const struct device *dev, struct uart_event *evt, void *user_data)
{
	
  int8_t rc;
	LOG_DBG("EVENT: %d", evt->type);

	switch (evt->type)
	{
	case UART_TX_DONE:
		setFlag(LIN::TXDONE);
    clearFlag(LIN::RXDONE);
		break;
  
	case UART_RX_BUF_REQUEST:
	case UART_RX_BUF_RELEASED:
	case UART_RX_DISABLED:
		break;
	case UART_RX_RDY:
    rc = uart_rx_buf_rsp(getDevice(), rxBuffer,sizeof(rxBuffer));
		setFlag(LIN::RXDONE);
    clearFlag(LIN::RXERROR);
		break;    
	default:
		LOG_WRN("Unhandled event %d", evt->type);
	}
}