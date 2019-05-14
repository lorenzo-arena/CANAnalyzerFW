/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "ble.h"
#include "dispatcher.h"
#include "canspy.h"
#include "mailformats.h"
#include "fatfs.h"
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
osThreadId bleTaskHandle;
osThreadId dispatcherTaskHandle;
osThreadId canLine1TaskHandle;

osMailQDef(commandMailHandle, 1, mailCommand);
osMailQId commandMailHandle;
osMailQDef(commandResponseMailHandle, 1, mailCommandResponse);
osMailQId commandResponseMailHandle;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

extern void MX_FATFS_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */

  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */

	commandMailHandle = osMailCreate(osMailQ(commandMailHandle), NULL);
	commandResponseMailHandle = osMailCreate(osMailQ(commandResponseMailHandle), NULL);

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
	osThreadDef(dispatcherTask, StartDispatcherTask, osPriorityNormal, 0, 512);
	dispatcherTaskHandle = osThreadCreate(osThread(dispatcherTask), NULL);

	osThreadDef(bleTask, StartBLETask, osPriorityNormal, 0, 1024);
	bleTaskHandle = osThreadCreate(osThread(bleTask), NULL);

	osThreadDef(canLine1Task, StartCANSpyTask, osPriorityNormal, 0, 128);
	canLine1TaskHandle = osThreadCreate(osThread(canLine1Task), (void *)1);
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
FATFS myFatFS;
FIL myFile;
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for FATFS */
  MX_FATFS_Init();

  /* USER CODE BEGIN StartDefaultTask */
	if(f_mount(&myFatFS, "", 1) != FR_OK)
	{
		Error_Handler();
	}
	/*
	FILINFO fileInfo;
	DIR dirInfo;
	FRESULT searchResult;
	int fileIndex = 0;

	searchResult = f_findfirst(&dirInfo, &fileInfo, "\\CAN1", "CAN1*.*");

	while(searchResult == FR_OK && fileInfo.fname[0])
	{
		fileIndex++;
		searchResult = f_findnext(&dirInfo, &fileInfo);
	}
	
	f_closedir(&dirInfo);
	
	
	
	char myFileName[] = "Test1.txt";
		
	f_mkdir("\\CAN1");
	
	f_mkdir("\\CAN2");
	
	f_mkdir("\\K");
	
	
	if(f_open(&myFile, myFileName, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
	{
		char myData[] = "Hello hello !";
		uint32_t myBytes;
		if(f_write(&myFile, myData, sizeof(myData) - 1, &myBytes) == FR_OK)
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		}

		f_close(&myFile);
	}
	
	*/

  /* Infinite loop */
  for(;;) { }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
