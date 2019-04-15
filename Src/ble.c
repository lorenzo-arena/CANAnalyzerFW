/**
  ******************************************************************************
  * @file           : ble.c
  * @brief          : BLE Driver for AT-09 module
  ******************************************************************************
  */
	
#include "ble.h"

#include "errors.h"
#include "commands.h"
#include "usart.h"
#include "crc.h"
#include "debug.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"

#define FRAME_HEADER "DSCA"

/* Private functions */
void InitBLE(void);
void MessageDispatcher(uint16_t commandGroup, uint16_t commandCode, uint8_t *dataBuff, uint32_t dataLength);
void SendCommandAndReceive(char * message);
void SendToModule_NOIT(char * message, int maxTimeout);
void SendToModule_IT(uint8_t *message, unsigned long maxLength);
void ReceiveFromModule_NOIT(char * message, unsigned long maxLength, unsigned int maxTimout);
void ReceiveFromModule_IT(uint8_t *message, unsigned long maxLength);
bool ReceiveInitCommand(uint32_t *nextLength, uint32_t *errorCode);
bool ReceiveCommand(uint32_t nextLength, uint32_t *errorCode);
/*********************/

/**
  * @brief  Function implementing the bleTask thread.
  * @param  argument: Not used 
  * @retval None
  */
void StartBLETask(void const * argument)
{	
	InitBLE();
	
	// Ripulisco il buffer
	HAL_UART_Init(&huart1);

	for(;;)
	{
		uint32_t errorCode = NO_ERROR;
		uint32_t nextLength = 0;
		
		if(ReceiveInitCommand(&nextLength, &errorCode))
		{
			ReceiveCommand(nextLength, &errorCode);
		}
		
		// Altrimenti pensare a come dare errore
	}
}

void InitBLE(void)
{	
	PrintLnDebugMessage("InitBLE");

	SendCommandAndReceive("AT");
	osDelay(10);
	// Importante: oltre al reset SW sarebbe utile fare in modo di effettuare un reset FW
	SendCommandAndReceive("AT+RESET");
	osDelay(10);
	SendCommandAndReceive("AT+ROLE0");
	osDelay(10);
	SendCommandAndReceive("AT+UUID0xFFE0");
	osDelay(10);
	SendCommandAndReceive("AT+CHAR0xFFE1");
	osDelay(10);
	SendCommandAndReceive("AT+NAMECANAnalyzer");
	osDelay(10);
	SendCommandAndReceive("AT+RESET");
	osDelay(10);
}

bool ReceiveInitCommand(uint32_t *nextLength, uint32_t *errorCode)
{
	const int initLength = 12;
	uint8_t initFrame[initLength];
	const int initResponseLength = 8;
	uint8_t initResponseFrame[initResponseLength];

	PrintLnDebugMessage("Receiving Init Frame..");
	ReceiveFromModule_IT(initFrame, initLength);
	
	if(strncmp((char *)initFrame, FRAME_HEADER, 4) == 0)
	{
		uint32_t crcInitCalc = 0;

		uint32_t lengthDataNext = (initFrame[4] << 24) |
					(initFrame[5] << 16) |
					(initFrame[6] << 8)  |
					(initFrame[7]);
						
		uint32_t crcInitSent = (initFrame[8] << 24) |
					(initFrame[9] << 16) |
					(initFrame[10] << 8)  |
					(initFrame[11]);
		
		// Controllo il CRC del frame di init, escludendo i byte del CRC
		crcInitCalc = CRC32_Compute(initFrame, initLength - 4);
		
		if(crcInitSent == crcInitCalc)
		{
			*nextLength = lengthDataNext;
			
			// Invio la risposta affermativa
			strcpy((char *)initResponseFrame, FRAME_HEADER);
			memcpy(initResponseFrame + 4, &errorCode, sizeof(uint32_t));
			
			SendToModule_IT(initResponseFrame, initResponseLength);
		}
		else
			*errorCode = CRC_ERROR;
	}
	else
		*errorCode = MARKER_ERROR;

	if(*errorCode != NO_ERROR)
		return false;
	
	return true;
}

bool ReceiveCommand(uint32_t length, uint32_t *errorCode)
{
	uint32_t commandLength = length;
	uint8_t *frame = NULL;

	// Aggiungo i 4 byte per il marker
	// e i 4 byte per il Crc32
	commandLength += 8;
	frame = malloc(commandLength);
	if(frame != NULL)
	{
		uint32_t crcInitSent = 0;
		uint32_t crcInitCalc = 0;

		PrintLnDebugMessage("Receiving Command Frame..");
		ReceiveFromModule_IT(frame, commandLength);
		
		if(strncmp((char *)frame, FRAME_HEADER, 4) == 0)
		{
		
			crcInitSent = (frame[commandLength - 4] << 24) |
										(frame[commandLength - 3] << 16) |
										(frame[commandLength - 2] << 8)  |
										(frame[commandLength - 1]);
			
			// Controllo il CRC del frame di init, escludendo i byte del CRC
			crcInitCalc = CRC32_Compute(frame, commandLength - 4);
			
			if(crcInitSent == crcInitCalc)
			{
				uint16_t group = *((uint16_t *)&frame[4]);
				uint16_t command = *((uint16_t *)&frame[6]);
				
				if(commandLength <= 12)
					MessageDispatcher(group, command, NULL, 0);
				else
					MessageDispatcher(group, command, &frame[8], commandLength - 12);
			}
			else
				*errorCode = CRC_ERROR;		
		}
		else
			*errorCode = MARKER_ERROR;
	}
	else
		*errorCode = MEMORY_ERROR;
	
	if(*errorCode != NO_ERROR)
		return false;
	
	return true;
}

void MessageDispatcher(uint16_t commandGroup, uint16_t commandCode, uint8_t *dataBuff, uint32_t dataLength)
{
	switch(commandGroup)
	{
		case GRP_TEST:
			if(commandCode == CMD_TEST)
			{
				HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
			}
			break;
	}
}

void SendCommandAndReceive(char * message)
{
	const int maxTimeout = 400;
	const int maxLength = 100;
	char reply[maxLength];
	memset(reply, 0x00, maxLength);

	// Loggo il comando inviato
	PrintDebugMessage("Sending: ");
	PrintLnDebugMessage(message);
	
	SendToModule_NOIT(message, maxTimeout);
	
	ReceiveFromModule_NOIT(reply, maxLength, maxTimeout);
	
	// Loggo la risposta ricevuta
	if(strlen(reply) != 0)
	{
		PrintDebugMessage("Received: ");
		PrintLnDebugMessage(reply);
	}
}

void SendToModule_IT(uint8_t *message, unsigned long maxLength)
{
	// Loggo il messaggio inviato
	PrintDebugMessage("Sending: ");
	PrintLnDebugBuffer(message, maxLength);
	
	HAL_UART_Transmit_IT(&huart1, message, maxLength);
	osSignalWait(UART1MessageSentSignal, osWaitForever);
}

void ReceiveFromModule_IT(uint8_t *message, unsigned long maxLength)
{
	HAL_UART_Receive_IT(&huart1, message, maxLength);
	osSignalWait(UART1MessageReceivedSignal, osWaitForever);
	
	// Loggo la risposta ricevuta
	PrintDebugMessage("Received: ");
	PrintLnDebugBuffer(message, maxLength);
}

void SendToModule_NOIT(char * message, int maxTimeout)
{
	char * toSend;
	toSend = malloc(strlen(message) + 3);
	strcpy(toSend, message);
	strcat(toSend, "\r\n");
	
	HAL_UART_Transmit(&huart1, (uint8_t*)toSend, strlen(toSend), maxTimeout);

	free(toSend);
}

void ReceiveFromModule_NOIT(char * message, unsigned long maxLength, unsigned int maxTimout)
{
	HAL_UART_Receive(&huart1, (uint8_t*)message, maxLength, maxTimout);
}



