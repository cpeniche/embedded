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

#include "can.h"
#include "can.cpp"
#include "mcu.h"
#include "spi.cpp"
#include "scheduler.h"
#include "halIncludes.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "motor.h"
#include "main.h"

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
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
//static void MX_USART2_UART_Init(void);
//static void MX_CAN_Init(void);
//static void MX_SPI1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 8;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 40;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 20;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_3|GPIO_PIN_5||GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB0 PB1 PB3 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_3|GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM3 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM3) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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

/************* vSpiTask ***********/
static void vSpiTask(void *pvParameters)
{

   uint8_t rx_data[2] = {0xAA,0xAA};

   MCU* _mcu = MCU::get_mcu_instance();
   _mcu->spi->set_data_lenght(sizeof(rx_data)/sizeof(uint8_t));
   _mcu->spi->set_timeout(10);

  while(1){

    _mcu->spi->Read(rx_data);
    vTaskDelay(20/portTICK_PERIOD_MS);
  }

}

/************* vCanTask ***********/
void vCanTask(void *pvParameters)
{

  static MCU* mcu = MCU::get_mcu_instance();
  mcu->can->vCanTask();

}

/************* vLatchTask ***********/
void vLatchTask(void *pvParameters)
{

  eLATCHPOSITION peBuffer;
  xLatchQueue = xQueueCreateStatic(QUEUE_LENGTH,
                                    ITEM_SIZE,
                                    ucQueueStorageArea,
                                    &xStaticQueue );

  configASSERT(xLatchQueue);

  while(1)
  {
    
    if (xQueueReceive(xLatchQueue,&peBuffer,portMAX_DELAY)== pdPASS)
    {      
      if(eOPEN == peBuffer )
      {        
        xSetPinState(GPIOB,GPIO_PIN_6,GPIO_PIN_RESET);
      }
      else
      {        
        xSetPinState(GPIOB,GPIO_PIN_6,GPIO_PIN_SET);
      }
      xSetPinState(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
      vTaskDelay(100/portTICK_PERIOD_MS);

      xSetPinState(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
      
    }
  }
}


#ifdef __cplusplus
}
#endif


int main()
{

	
  MCU* mcu = MCU::get_mcu_instance();
	HSI_Clock *hsi_clock = new HSI_Clock();
	Spi<uint8_t,uint32_t> *spi = new Spi<uint8_t,uint32_t>();
	Can_Obj* can = new Can_Obj();
	

	HAL_Init();

	mcu->Set_Clock(*hsi_clock);
	mcu->clock->Init();
	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	mcu->Register_Spi(*spi);
	mcu->Register_Can(*can);
	mcu->spi->Init();
	mcu->can->driver.Init();

  /* Use to generate PWM to motors*/
  MX_TIM2_Init();

  xTaskCreate(vSpiTask,"Spi Task",
              configMINIMAL_STACK_SIZE,
              (void *)0,
              configMAX_PRIORITIES,
              NULL);         
  
  xTaskCreate(vCanTask,"Can Task",
              configMINIMAL_STACK_SIZE,
              (void *)0,
              configMAX_PRIORITIES,
              NULL);

  
  xTaskCreate(vLatchTask,"Latch Position Task",
              configMINIMAL_STACK_SIZE,
              (void *)0,
              configMAX_PRIORITIES,
              NULL);
       
  vTaskStartScheduler();

  while(1);
}







