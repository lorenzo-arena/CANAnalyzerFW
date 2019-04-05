/**
  ******************************************************************************
  * @file           : debug.c
  * @brief          : Basic debug logging functions
  ******************************************************************************
  */
	
#include "debug.h"
#include "main.h"
#include "usart.h"
#include "string.h"
#include "stdlib.h"

void PrintDebugMessage(const char * message)
{
	HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
}

void PrintLnDebugMessage(const char * message)
{
	char * toSend;
	toSend = malloc(strlen(message) + 3);
	strcpy(toSend, message);
	strcat(toSend, "\r\n");
	
	HAL_UART_Transmit(&huart2, (uint8_t*)toSend, strlen(toSend), HAL_MAX_DELAY);

	free(toSend);
}

