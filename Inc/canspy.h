/**
  ******************************************************************************
  * File Name          : canspy.h
  * Description        : This file provides code for the CAN spy line.
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CANSPY_H
#define __CANSPY_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

#include "stdbool.h"

typedef struct
{
	uint32_t bitTiming;
	uint32_t frameFormat;
	bool errorReception;
	bool applyMaskAndId;
	uint32_t mask;
	uint32_t id;
} CANSpyParam;

void StartCANSpyTask(void const * argument);
void SetCANLineParameter(int lineNumber, CANSpyParam params);

#endif /*__CANSPY_H */

