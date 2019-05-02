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
	
static const uint32_t NO_ERROR = 0x00000000;

// Communication errors
static const uint32_t COMMUNICATION_ERROR = 0x0001;

static const uint32_t TIMEOUT_ERROR = ((COMMUNICATION_ERROR << 16) | 0x00000000);
static const uint32_t MARKER_ERROR = ((COMMUNICATION_ERROR << 16) | 0x00000001);
static const uint32_t CRC_ERROR = ((COMMUNICATION_ERROR << 16) | 0x00000002);
static const uint32_t MEMORY_ERROR = ((COMMUNICATION_ERROR << 16) | 0x00000003);
	
// Command Errors
static const uint32_t COMMAND_ERROR = 0x0002;

static const uint32_t GROUP_NOT_VALID_ERROR = ((COMMAND_ERROR << 16) | 0x00000000);
static const uint32_t COMMAND_NOT_VALID_ERROR = ((COMMAND_ERROR << 16) | 0x00000001);
static const uint32_t COMMAND_NOT_EXECUTED_ERROR = ((COMMAND_ERROR << 16) | 0x00000002);
static const uint32_t PARAMETERS_NOT_CORRECT = ((COMMAND_ERROR << 16) | 0x00000003);

// Firmware Errors
static const uint32_t FIRMWARE_ERROR = 0x0003;

static const uint32_t ERROR_FROM_HANDLER = ((FIRMWARE_ERROR << 16) | 0x00000000);

#endif

