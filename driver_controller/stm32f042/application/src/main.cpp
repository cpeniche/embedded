/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mcu.h"
#include "scheduler.h"
#include "stm32f0xx_hal_can.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
//CAN_HandleTypeDef hcan;

//SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CAN_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
#if 0
int main(void)
{

	/* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_CAN_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
#endif

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
#if 0
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN;
  hcan.Init.Prescaler = 16;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_1TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}
#endif


/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 38400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}



#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */


static void Spi_task()
{

  uint8_t rx_data[2] = {0xAA,0xAA};

  MCU* _mcu = MCU::get_mcu_instance();
  _mcu->spi->set_data_lenght(sizeof(rx_data)/sizeof(uint8_t));
  _mcu->spi->set_timeout(10);
  _mcu->spi->Read(rx_data);

}


static void Can_task()
{

#if 0
  uint8_t *data;

  static MCU* _mcu = MCU::get_mcu_instance();
  static Can_Rx_Msg<CAN_RxHeaderTypeDef> rx_driver_module;
//  static Can_Tx_Msg<CAN_TxHeaderTypeDef> tx_driver_module;
//
//  tx_driver_module.data[0]=0xAA;
//  tx_driver_module.data[1]=0xBB;
//  tx_driver_module.data[2]=0xCC;
//  tx_driver_module.data[3]=0xDD;
  //_mcu->can->driver.Write(tx_driver_module);


  _mcu->can->driver.Read(rx_driver_module);

  if(_mcu->can->driver.GetError() != 0x1 )
  {
    data = rx_driver_module.get_data_ptr();
  }
#endif
}

#ifdef __cplusplus
}
#endif


int main()
{

	MCU* mcu = MCU::get_mcu_instance();
	HSI_Clock *hsi_clock = new HSI_Clock();
	//Spi<uint8_t,uint32_t> *spi = new Spi<uint8_t,uint32_t>();
	Can_Obj* can = new Can_Obj();
	uint32_t tickstart = 0;

	HAL_Init();

	mcu->Set_Clock(*hsi_clock);
	mcu->clock->Init();
	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	//mcu->Register_Spi(*spi);
	mcu->Register_Can(*can);
	//mcu->spi->Init();
	mcu->can->driver.Init();

	Scheduler *sched = new Scheduler();
	//Task *spi_task=new Task(&Spi_task,1000,20);
	Task *can_task=new Task(&Can_task,500,0);
	//sched->add_task(*spi_task);
	sched->add_task(*can_task);

	do
	{
	  sched->exec();
	}while(1);
}


extern "C" void CEC_CAN_IRQHandler()
{

  MCU* mcu = MCU::get_mcu_instance();

  mcu->can->driver.Call_Interrupt_Handle();
}


extern "C" void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{

  MCU* mcu = MCU::get_mcu_instance();
  Can_Rx_Msg<CAN_RxHeaderTypeDef> rx_msg;
  mcu->can->driver.Read(rx_msg);
  mcu->can->driver.QueueRxMessage(rx_msg);

}


