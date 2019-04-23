/**
  ******************************************************************************
  * @file           : errors.c
  * @brief          : Error defines for CAN Analyzer
  ******************************************************************************
  */
	
#include "errors.h"

const uint32_t NO_ERROR = 0x00000000;

// Communication errors
const uint32_t COMMUNICATION_ERROR = 0x0001;

const uint32_t TIMEOUT_ERROR = ((COMMUNICATION_ERROR << 16) | 0x00000001);
const uint32_t MARKER_ERROR = ((COMMUNICATION_ERROR << 16) | 0x00000002);
const uint32_t CRC_ERROR = ((COMMUNICATION_ERROR << 16) | 0x00000003);
const uint32_t MEMORY_ERROR = ((COMMUNICATION_ERROR << 16) | 0x00000004);
	
// Command Errors
const uint32_t COMMAND_ERROR = 0x0002;

const uint32_t GROUP_NOT_VALID_ERROR = ((COMMAND_ERROR << 16) | 0x00000001);
const uint32_t COMMAND_NOT_VALID_ERROR = ((COMMAND_ERROR << 16) | 0x00000002);

