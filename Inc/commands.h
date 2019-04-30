/**
  ******************************************************************************
  * File Name          : commands.h
  * Description        : This file provides code for the CANAnalyzer commands definitions.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMANDS_H
#define __COMMANDS_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal.h"

static const uint16_t GRP_TEST = 0x3F3F;
static const uint16_t CMD_TEST = 0x3F3F;

static const uint16_t GRP_INFO = 0x0000;
static const uint16_t CMD_GETSERIALNUMBER = 0x0001;
static const uint16_t CMD_GETFIRMWAREVERSION = 0x0002;

static const uint16_t GRP_CAN_LINE1 = 0x0001;
static const uint16_t GRP_CAN_LINE2 = 0x0002;
static const uint16_t CMD_STARTCANLINE = 0x0001;
static const uint16_t CMD_STOPCANLINE = 0x0002;
static const uint16_t CMD_SETCANPARAM = 0x0003;

static const uint16_t GRP_K_LINE = 0x0003;

#endif



