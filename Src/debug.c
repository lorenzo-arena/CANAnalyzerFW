/**
  ******************************************************************************
  * @file           : debug.c
  * @brief          : Basic debug logging functions
  ******************************************************************************
  */
	
#include "debug.h"
#include "usart.h"
#include "cexception.h"
#include "errors.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

void PrintDebugMessage(const char * message)
{
	HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
}

void PrintLnDebugMessage(const char * message)
{
	char * toSend;
	
	toSend = malloc(strlen(message) + 3);
	if(toSend == NULL)
		Throw(MEMORY_ERROR);
	
	strcpy(toSend, message);
	strcat(toSend, "\r\n");
	
	HAL_UART_Transmit(&huart2, (uint8_t*)toSend, strlen(toSend), HAL_MAX_DELAY);

	free(toSend);
}

void PrintLnDebugBuffer(uint8_t * buff, uint32_t buffLength)
{
	char * toSend;
	int charIndex = 0;
	
	toSend = malloc((buffLength * 2));
	if(toSend == NULL)
		Throw(MEMORY_ERROR);
	
	memset(toSend, 0x00, buffLength * 2);
	
	for(charIndex = 0; charIndex < buffLength; charIndex ++)
	{
		sprintf(&toSend[charIndex * 2], "%.2X", buff[charIndex]);
	}
	
	strcat(toSend, "\r\n");
	
	HAL_UART_Transmit(&huart2, (uint8_t*)toSend, strlen(toSend), HAL_MAX_DELAY);

	free(toSend);
}

