/**
  ******************************************************************************
  * File Name          : CAN.c
  * Description        : This file provides code for the configuration
  *                      of the CAN instances.
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

/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
#include "main.h"
#include "string.h"

uint32_t CAN1BufferHead = 0;
uint32_t CAN1BufferTail = 0;
CANMsg CAN1SpyBuffer[CANSpyBufferLength];

uint32_t CAN2BufferHead = 0;
uint32_t CAN2BufferTail = 0;
CANMsg CAN2SpyBuffer[CANSpyBufferLength];
/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 40;
  hcan1.Init.Mode = CAN_MODE_SILENT;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_7TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }

}
/* CAN2 init function */
void MX_CAN2_Init(void)
{

  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 40;
  hcan2.Init.Mode = CAN_MODE_SILENT;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_7TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }

}

static uint32_t HAL_RCC_CAN1_CLK_ENABLED=0;

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }
  
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN1 GPIO Configuration    
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_TX_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
    HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspInit 0 */

  /* USER CODE END CAN2_MspInit 0 */
    /* CAN2 clock enable */
    __HAL_RCC_CAN2_CLK_ENABLE();
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }
  
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN2 GPIO Configuration    
    PB12     ------> CAN2_RX
    PB13     ------> CAN2_TX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN2 interrupt Init */
    HAL_NVIC_SetPriority(CAN2_TX_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_SetPriority(CAN2_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN2_RX1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX1_IRQn);
    HAL_NVIC_SetPriority(CAN2_SCE_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN2_SCE_IRQn);
  /* USER CODE BEGIN CAN2_MspInit 1 */

  /* USER CODE END CAN2_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }
  
    /**CAN1 GPIO Configuration    
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX 
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_SCE_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspDeInit 0 */

  /* USER CODE END CAN2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN2_CLK_DISABLE();
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }
  
    /**CAN2 GPIO Configuration    
    PB12     ------> CAN2_RX
    PB13     ------> CAN2_TX 
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13);

    /* CAN2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN2_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN2_RX1_IRQn);
    HAL_NVIC_DisableIRQ(CAN2_SCE_IRQn);
  /* USER CODE BEGIN CAN2_MspDeInit 1 */

  /* USER CODE END CAN2_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN_RxHeaderTypeDef   RxHeader;
	uint8_t               RxData[8];
	CANMsg *frame;
	uint32_t *bufferTail;
	uint32_t *bufferHead;
	uint32_t actualSize = 0;
	
	if(hcan->Instance == CAN1)
	{
		bufferHead = &CAN1BufferHead;
		bufferTail = &CAN1BufferTail;
		frame = &CAN1SpyBuffer[CAN1BufferTail];
		HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, RxData);
	}
	else if(hcan->Instance == CAN2)
	{
		bufferHead = &CAN2BufferHead;
		bufferTail = &CAN2BufferTail;
		frame = &CAN2SpyBuffer[CAN2BufferTail];
		HAL_CAN_GetRxMessage(&hcan2, CAN_RX_FIFO0, &RxHeader, RxData);
	}
	
	// In base all'indice dell'ultimo messaggio aggancio al buffer della spia un messaggio	
	// 1 - TODO : impostare campo time

	// 2 - Imposto l'ID del messaggio ricevuto
	frame->id = RxHeader.StdId;
	
	// 3 - Copio i dati del buffer
	frame->dataSize = RxHeader.DLC;
	memcpy(frame->data, RxData, RxHeader.DLC);
	
	// 4 - Non si tratta di un errore
	frame->isError = 0x00;
	frame->errorCode = 0x00000000;
	
	// 5 - Incremento il puntatore alla coda
	(*bufferTail)++;
	if( (*bufferTail) >= CANSpyBufferLength )
		(*bufferTail) -= CANSpyBufferLength;
	
	// Calcolo il numero di messaggi attuali per lanciare il salvataggio
	if( (*bufferTail) >= (*bufferHead) )
		actualSize = (*bufferTail) - (*bufferHead);	
	else
		actualSize = CANSpyBufferLength + (*bufferTail) - (*bufferHead);
	
	if(actualSize >= CANSpyBufferLengthToFlush)
	{
		if(hcan->Instance == CAN1)
		{
			osSignalSet(canLine1TaskHandle, CANBufferHasToBeFlushed);
		}
		else if(hcan->Instance == CAN2)
		{
			osSignalSet(canLine2TaskHandle, CANBufferHasToBeFlushed);
		}
	}
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
	uint8_t errorCode;
	CANMsg *frame;
	uint32_t *bufferTail;
	uint32_t *bufferHead;
	uint32_t actualSize = 0;

	errorCode = HAL_CAN_GetError(hcan);
	HAL_CAN_ResetError(hcan);	
	
	if(hcan->Instance == CAN1)
	{
		bufferHead = &CAN1BufferHead;
		bufferTail = &CAN1BufferTail;
		frame = &CAN1SpyBuffer[CAN1BufferTail];
	}
	else if(hcan->Instance == CAN2)
	{
		bufferHead = &CAN2BufferHead;
		bufferTail = &CAN2BufferTail;		
		frame = &CAN2SpyBuffer[CAN2BufferTail];
	}
	
	// In base all'indice dell'ultimo messaggio aggancio al buffer della spia un messaggio
	
	// 1 - TODO : impostare campo time
	
	// 2 - Si tratta di un errore
	frame->isError = 0x01;
	frame->errorCode = errorCode;
	
	// 3 - Incremento il puntatore alla coda
	(*bufferTail)++;
	if( (*bufferTail) >= CANSpyBufferLength )
		(*bufferTail) -= CANSpyBufferLength;
	
	
	// Calcolo il numero di messaggi attuali per lanciare il salvataggio
	if( (*bufferTail) >= (*bufferHead) )
		actualSize = (*bufferTail) - (*bufferHead);	
	else
		actualSize = CANSpyBufferLength + (*bufferTail) - (*bufferHead);
	
	if(actualSize >= CANSpyBufferLengthToFlush)
	{
		if(hcan->Instance == CAN1)
		{
			osSignalSet(canLine1TaskHandle, CANBufferHasToBeFlushed);
		}
		else if(hcan->Instance == CAN2)
		{
			osSignalSet(canLine2TaskHandle, CANBufferHasToBeFlushed);
		}
	}
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
