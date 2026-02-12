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
#include "inputInterface.h"
#include "lin.h"
LOG_MODULE_REGISTER(LIN, LOG_LEVEL_INF);

#define CALL_MEMBER_FN(object, ptrToMember) ((object)->*(ptrToMember))
callBackPtr callback;
void linCallBack(const struct device *dev, struct uart_event *evt, void *user_data);

LIN::LIN(struct device *dev, uint8_t *rxBuffer, size_t length, uint8_t Identifier, uart_callback_t func)
{
	setDevice(dev);
	setProtectedID(Identifier);
	this->rxBuffer = rxBuffer;
	this->buffLength = length;
	this->setCallback(func);
}

int8_t LIN::Transmit(uint8_t *buffer, size_t size)
{
	clearFlag(TXDONE);
	txBuffer[0] = SynchData;
	txBuffer[1] = (protectedId << 2) | IdentifierFieldParity(protectedId);
	if (buffer != NULL)
		for (size_t idx = 2; idx < size; idx++)
			txBuffer[idx] = buffer[idx - 2];
	error = uart_tx(this->dev, txBuffer, size, SYS_FOREVER_US);
	if (error != 0)
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
	flags |= 1 << flag;
}

void LIN::clearFlag(eFlags flag)
{
	flags &= ~(1 << flag);
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

	LOG_DBG("EVENT: %d", evt->type);

	switch (evt->type)
	{
	case UART_TX_DONE:
		setFlag(LIN::TXDONE);
		clearFlag(LIN::RXDONE);
		break;
	case UART_RX_BUF_REQUEST:
		error = uart_rx_buf_rsp(dev, (rxBuffer + (buffLength * idxBuffer)), buffLength);
		__ASSERT_NO_MSG(error == 0);
		idxBuffer = idxBuffer ? 0 : 1;
		break;
	case UART_RX_BUF_RELEASED:
	case UART_RX_DISABLED:
		break;
	case UART_RX_RDY:
		setFlag(LIN::RXDONE);
		rxReadyDataPtr = evt->data.rx.buf + evt->data.rx.offset;
		if (evt->data.rx.len != buffLength)
			setFlag(LIN::RXERROR);
		else
		{
			if (CalculateChecksum((rxReadyDataPtr + 1), buffLength - 2) != rxReadyDataPtr[6])
				setFlag(LIN::RXERROR);
		}
		break;
	case UART_RX_STOPPED:
		setFlag(LIN::RXSTOP);
		setFlag(LIN::RXDONE);
		break;
	default:
		LOG_WRN("Unhandled event %d", evt->type);
	}
}

int8_t LIN::enableReceive()
{
	clearFlag(LIN::RXSTOP);
	return error = uart_rx_enable(dev, rxBuffer, buffLength, 100);
}

int8_t LIN::readInput(uint8_t *buffer, size_t size)
{
	return error = Transmit(buffer, size);
}

int8_t LIN::getInput(uint8_t *buffer)
{

	if ((getFlags() & (1 << RXERROR)) != 0)
	{
		error = -1;
		buffer = nullptr;
	}
	else
	{
		memcpy(buffer, rxReadyDataPtr+1, this->buffLength - 1);
		error = 0;
	}
	return error;
}

bool LIN::isDataReady(void)
{
	if ((getFlags() & (1 << RXDONE)) != 0)
		return true;
	else
		return false;
}

int8_t LIN::getError(void)
{
	return error;
}

uint8_t LIN::CalculateChecksum(uint8_t *ptr, size_t length)
{
	uint16_t chkSum = 0;

	for (uint8_t idx = 0; idx < length; idx++)
		chkSum += ptr[idx];
	return (~(uint8_t)chkSum) - 1;
}
