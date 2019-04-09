/**
  ******************************************************************************
  * File Name          : debug.h
  * Description        : This file provides code for the debug logging.
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DEBUG_H
#define __DEBUG_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal.h"

void PrintDebugMessage(const char * message);
void PrintLnDebugMessage(const char * message);
void PrintLnDebugBuffer(uint8_t * buff, uint32_t buffLength);

#endif /*__ can_H */
