/**
  ******************************************************************************
  * File Name          : ble.h
  * Description        : This file provides code for the BLE communication.
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BLE_H
#define __BLE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

void StartBLETask(void const * argument);

#endif /*__BLE_H */
