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

void StartCANSpyTask(void const * argument);

#endif /*__CANSPY_H */

