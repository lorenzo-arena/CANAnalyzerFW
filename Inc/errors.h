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
	
const uint32_t NO_ERROR = 0x00000000;

// Frame errors
const uint32_t FRAME_ERROR = 0x0001;

const uint32_t MARKER_ERROR = ((FRAME_ERROR << 16) | 0x00000001);
const uint32_t CRC_ERROR = ((FRAME_ERROR << 16) | 0x00000002);
const uint32_t MEMORY_ERROR = ((FRAME_ERROR << 16) | 0x00000003);
	
// Command Errors
const uint32_t COMMAND_ERROR = 0x0002;

const uint32_t GROUP_NOT_VALID_ERROR = ((COMMAND_ERROR << 16) | 0x00000001);
const uint32_t COMMAND_NOT_VALID_ERROR = ((COMMAND_ERROR << 16) | 0x00000002);

#endif

