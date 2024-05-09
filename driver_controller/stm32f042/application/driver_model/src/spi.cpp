/*
 * Spi.cpp
 *
 *  Created on: May 8, 2024
 *      Author: carlo
 */

#include "spi.h"


static void MX_SPI1_Init(void *drv);

void Spi::Init()
{
	MX_SPI1_Init((void *)&drv_handle);
}

void Spi::Read ()
{
}

void Spi::Write ()
{
}


SPI_HandleTypeDef *Spi::get_handle()
{
	return &drv_handle;
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */

static void MX_SPI1_Init(void *drv)
{

	SPI_HandleTypeDef *hspi1 = (SPI_HandleTypeDef *)drv;

  hspi1->Instance = SPI1;
  hspi1->Init.Mode = SPI_MODE_MASTER;
  hspi1->Init.Direction = SPI_DIRECTION_2LINES_RXONLY;
  hspi1->Init.DataSize = SPI_DATASIZE_4BIT;
  hspi1->Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1->Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1->Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi1->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1->Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1->Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1->Init.CRCPolynomial = 7;
  hspi1->Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1->Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(hspi1) != HAL_OK)
  {
//    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */
}
