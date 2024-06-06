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
#include "motor.h"
#include "FreeRTOS.h"
#include "task.h"

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
TIM_HandleTypeDef htim3;


#define mainQUEUE_RECEIVE_TASK_PRIORITY     ( tskIDLE_PRIORITY + 2 )

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CAN_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */

static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 4800;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 2400;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

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
    //Error_Handler();
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
GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB3 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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


static void Can_task(void *pvParameters)
{
  static MCU* mcu = MCU::get_mcu_instance();
  Can_Rx_Msg<CAN_RxHeaderTypeDef> rx_msg;
  
  while(mcu->can->driver.Get_RxQueue_Size() != 0)
  {
    rx_msg=mcu->can->driver.Get_Queue_Msg();
    for(std::list<Dictionary>:: iterator it=dictionary.begin();
        it != dictionary.end(); it++)
    {
       if ((rx_msg.header.StdId & 0x7FF) == (it->Module_Id & 0x7FF))
       {
          
          switch(it->dataType)
          {
            case UINT8:            
            *((uint8_t *)it->data_ptr)=rx_msg.data[0];
            break;

            case UINT16:
            *((uint16_t *)it->data_ptr)=*((uint8_t *)(rx_msg.data));
            break;
          
            default:
              break;
          }          
          it->Callback(WRITE);
       }
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
  MX_TIM3_Init();

  xTaskCreate(Can_task,                   /* The function that implements the task. */
              "Can Recevie Task",                                   /* The text name assigned to the task - for debug only as it is not used by the kernel. */
              4 * configMINIMAL_STACK_SIZE,           /* The size of the stack to allocate to the task. */
              NULL, /* The parameter passed to the task - just to check the functionality. */
              mainQUEUE_RECEIVE_TASK_PRIORITY,        /* The priority assigned to the task. */
              NULL );       
  /* Start the tasks and timer running. */
  vTaskStartScheduler();
  while(1);

#if 0
	Scheduler *sched = new Scheduler();
	//Task *spi_task=new Task(&Spi_task,1000,20);
	Task *can_task=new Task(&Can_task,10,0);
  Task *motor_task=new Task(&Motor_task,200,20);
	//sched->add_task(*spi_task);
	sched->add_task(*can_task);
  sched->add_task(*motor_task);

	do
	{
	  sched->exec();
	}while(1);
#endif


}


/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    /* If configCHECK_FOR_STACK_OVERFLOW is set to either 1 or 2 then this
     * function will automatically get called if a task overflows its stack. */
    ( void ) pxTask;
    ( void ) pcTaskName;
    for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
    /* If configUSE_MALLOC_FAILED_HOOK is set to 1 then this function will
     * be called automatically if a call to pvPortMalloc() fails.  pvPortMalloc()
     * is called automatically when a task, queue or semaphore is created. */
    for( ;; );
}

/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task. */
extern void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
     * state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task. */
extern void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
 * function then they must be declared static - otherwise they will be
 * allocated on the stack and so not exists after this function exits. */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
/*-----------------------------------------------------------*/



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


