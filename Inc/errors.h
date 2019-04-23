/**
  ******************************************************************************
  * @file           : errors.h
  * @brief          : Error codes for CAN Analyzer
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ERRORS_H
#define __ERRORS_H
	
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal.h"
	
extern const uint32_t NO_ERROR;

// Communication errors
extern const uint32_t COMMUNICATION_ERROR;

extern const uint32_t TIMEOUT_ERROR;
extern const uint32_t MARKER_ERROR;
extern const uint32_t CRC_ERROR;
extern const uint32_t MEMORY_ERROR;
	
// Command Errors
extern const uint32_t COMMAND_ERROR;

extern const uint32_t GROUP_NOT_VALID_ERROR;
extern const uint32_t COMMAND_NOT_VALID_ERROR;

#endif

