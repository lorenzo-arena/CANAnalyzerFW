/**
  ******************************************************************************
  * @file           : mailformats.h
  * @brief          : Mail formats to communicate between threads
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAILFORMATS_H
#define __MAILFORMATS_H
	
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal.h"

typedef struct
{
	uint16_t group;
	uint16_t code;
} mailCommand;

typedef struct
{
	uint32_t errorCode;
	uint32_t response;
	uint32_t needBuffer;
} mailCommandResponse;

#endif
