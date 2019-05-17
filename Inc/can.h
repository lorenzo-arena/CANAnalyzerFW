/**
  ******************************************************************************
  * File Name          : CAN.h
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __can_H
#define __can_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

/* USER CODE BEGIN Private defines */
#define CANMessageReceivedSignal 0x0001

#define NORMAL_FRAME_FORMAT 11
#define EXTENDED_FRAME_FORMAT 29

#define CANSpyBufferLength	250

// Dimensione totale: 20 byte
typedef struct
{
	uint32_t time; // utilizzare ?
	uint32_t id;
	uint8_t dataSize;
	uint8_t isError;
	uint8_t errorCode;
	uint8_t dummy; // per allineamento 4 byte	
	uint8_t data[8];
} CANMsg;

extern uint32_t CAN1BufferHead;
extern uint32_t CAN1BufferTail;
extern CANMsg CAN1SpyBuffer[CANSpyBufferLength];

extern uint32_t CAN2BufferHead;
extern uint32_t CAN2BufferTail;
extern CANMsg CAN2SpyBuffer[CANSpyBufferLength];

/* USER CODE END Private defines */

void MX_CAN1_Init(void);
void MX_CAN2_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ can_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
