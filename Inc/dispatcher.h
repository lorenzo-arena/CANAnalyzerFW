/**
  ******************************************************************************
  * File Name          : dispatcher.h
  * Description        : This file provides code for the message dispatching.
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DISPATCHER_H
#define __DISPATCHER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

void StartDispatcherTask(void const * argument);

#endif /*__DISPATCHER_H */

