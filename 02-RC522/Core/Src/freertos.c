/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "rc522.h"
#include <stdio.h>
#include <string.h>
#include "shell_port.h"

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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId LED_TaskHandle;
osThreadId RC522_TaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void Start_LED_Task(void const * argument);
void Start_RC522_Task(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
    *ppxIdleTaskStackBuffer = &xIdleStack[0];
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
    /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    /* USER CODE END RTOS_QUEUES */

    /* Create the thread(s) */
    /* definition and creation of LED_Task */
    osThreadDef(LED_Task, Start_LED_Task, osPriorityNormal, 0, 128);
    LED_TaskHandle = osThreadCreate(osThread(LED_Task), NULL);

    /* definition and creation of RC522_Task */
    osThreadDef(RC522_Task, Start_RC522_Task, osPriorityIdle, 0, 128);
    RC522_TaskHandle = osThreadCreate(osThread(RC522_Task), NULL);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_Start_LED_Task */
/**
  * @brief  Function implementing the LED_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Start_LED_Task */
void Start_LED_Task(void const * argument)
{
    /* USER CODE BEGIN Start_LED_Task */
    /* Infinite loop */
    for(;;)
    {
        HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
        HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
        osDelay(500);
    }
    /* USER CODE END Start_LED_Task */
}

/* USER CODE BEGIN Header_Start_RC522_Task */
/**
* @brief Function implementing the RC522_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_RC522_Task */
void Start_RC522_Task(void const * argument)
{
    /* USER CODE BEGIN Start_RC522_Task */
	
	uint8_t readUid[5]; 
	uint8_t UID[5]={0x73,0x44,0xa5,0x17};//自己的卡号，可以通过串口打印通过下面读取到的打印到上位机，再把那个读取的卡号填入数组
    /* Infinite loop */
    for(;;)
    {
			if(!readCard(readUid,NULL)){
			printf("卡号：%x-%x-%x-%x\n",readUid[0],readUid[1],readUid[2],readUid[3]);
			if(!strncmp((char *)readUid,(char *)UID,4)){
				//比对卡号正确要做什么
				printf("卡号正确\r\n");
				
			}
			else{
				//比对卡号错误要做什么
				printf("卡号错误\r\n");
				
			}
		}
			osDelay(500);
    }
    /* USER CODE END Start_RC522_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
