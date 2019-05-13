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
static const uint16_t CMD_BLINK = 0x0001;
static const uint16_t CMD_SLEEP = 0x0002;

static const uint16_t GRP_INFO = 0x0000;
static const uint16_t CMD_GETSERIALNUMBER = 0x0001;
static const uint16_t CMD_GETFIRMWAREVERSION = 0x0002;
static const uint16_t CMD_GETCAN1FILESNUM = 0x0003;
static const uint16_t CMD_GETCAN2FILESNUM = 0x0004;
static const uint16_t CMD_GETKFILESNUM = 0x0005;
static const uint16_t CMD_GETCAN1FILENAME = 0x0006;
static const uint16_t CMD_GETCAN2FILENAME = 0x0007;
static const uint16_t CMD_GETKFILENAME = 0x0008;
static const uint16_t CMD_GETCAN1FILE = 0x0009;
static const uint16_t CMD_GETCAN2FILE = 0x000A;
static const uint16_t CMD_GETKFILE = 0x000B;

static const uint16_t GRP_CAN_LINE1 = 0x0001;
static const uint16_t GRP_CAN_LINE2 = 0x0002;
static const uint16_t CMD_STARTCANLINE = 0x0001;
static const uint16_t CMD_STOPCANLINE = 0x0002;
static const uint16_t CMD_SETCANPARAM = 0x0003;

static const uint16_t GRP_K_LINE = 0x0003;

static const uint16_t GRP_UTILITY = 0x0004;
static const uint16_t CMD_WAIT = 0x0000;

#endif



