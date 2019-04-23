/**
  ******************************************************************************
  * @file           : arr_converter.h
  * @brief          : Functions to convert byte array to uint
  ******************************************************************************
  */
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ARR_CONVERTER_H
#define __ARR_CONVERTER_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal.h"

uint16_t GetUInt16FromBuffer(uint8_t *buff, uint32_t index);
uint32_t GetUInt32FromBuffer(uint8_t *buff, uint32_t index);
void SetBufferFromUInt16(uint16_t toSet, uint8_t *buff, uint32_t index);
void SetBufferFromUInt32(uint32_t toSet, uint8_t *buff, uint32_t index);

#endif
