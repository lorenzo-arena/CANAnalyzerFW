/**
  ******************************************************************************
  * @file           : ble.c
  * @brief          : BLE Driver for AT-09 module
  ******************************************************************************
  */
	
#include "ble.h"

#include "errors.h"
#include "commands.h"
#include "cexception.h"
#include "arr_converter.h"
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
bool ReceiveInitCommand(uint32_t *nextLength);
bool ReceiveCommand(uint32_t nextLength);
/*********************/

/**
  * @brief  Function implementing the bleTask thread.
  * @param  argument: Not used 
  * @retval None
  */
void StartBLETask(void const * argument)
{	
	CEXCEPTION_T ex;

	InitBLE();
	
	// Ripulisco il buffer
	HAL_UART_Init(&huart1);

	for(;;)
	{
		uint32_t nextLength = 0;
		
		Try
		{
			if(ReceiveInitCommand(&nextLength))
			{
				ReceiveCommand(nextLength);
			}
		}
		Catch(ex)
		{
			// Invio il messaggio di error
			uint8_t errorFrame[8];
			
			strcpy((char *)errorFrame, FRAME_HEADER);
			memcpy(errorFrame + 4, &ex, sizeof(uint32_t));
			SendToModule_IT(errorFrame, sizeof(errorFrame));
		}
	}
}

void InitBLE(void)
{	
	PrintLnDebugMessage("InitBLE");

	SendCommandAndReceive("AT");
	osDelay(10);
	// Importante: oltre al reset SW sarebbe utile fare in modo di effettuare un reset HW
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

bool ReceiveInitCommand(uint32_t *nextLength)
{
	const int initLength = 12;
	uint8_t initFrame[initLength];
	const int initResponseLength = 8;
	uint8_t initResponseFrame[initResponseLength];
	uint32_t crcInitCalc = 0;
	uint32_t lengthDataNext = 0;
	uint32_t crcInitSent = 0;

	PrintLnDebugMessage("Receiving Init Frame..");
	ReceiveFromModule_IT(initFrame, initLength);
	
	if(strncmp((char *)initFrame, FRAME_HEADER, 4) != 0)
		Throw(MARKER_ERROR);

	lengthDataNext = GetUInt32FromBuffer(initFrame, 4);
					
	crcInitSent = GetUInt32FromBuffer(initFrame, 8);
	
	// Controllo il CRC del frame di init, escludendo i byte del CRC
	crcInitCalc = CRC32_Compute(initFrame, initLength - 4);
	
	if(crcInitSent != crcInitCalc)
		Throw(CRC_ERROR);

	*nextLength = lengthDataNext;
	
	// Invio la risposta affermativa
	strcpy((char *)initResponseFrame, FRAME_HEADER);
	SetBufferFromUInt32(NO_ERROR, initResponseFrame, 4);
	
	SendToModule_IT(initResponseFrame, initResponseLength);		
	
	return true;
}

bool ReceiveCommand(uint32_t length)
{
	uint32_t commandLength = length;
	uint8_t *frame = NULL;
	uint32_t crcInitSent = 0;
	uint32_t crcInitCalc = 0;
	uint16_t group = 0;
	uint16_t command = 0;

	// Aggiungo i 4 byte per il marker
	// e i 4 byte per il Crc32
	commandLength += 8;
	frame = malloc(commandLength);
	if(frame == NULL)
		Throw(MEMORY_ERROR);

	PrintLnDebugMessage("Receiving Command Frame..");
	ReceiveFromModule_IT(frame, commandLength);
	
	if(strncmp((char *)frame, FRAME_HEADER, 4) != 0)
	{
		free(frame);
		Throw(MARKER_ERROR);
	}

	crcInitSent = GetUInt32FromBuffer(frame, commandLength - 4);
	
	// Controllo il CRC del frame di init, escludendo i byte del CRC
	crcInitCalc = CRC32_Compute(frame, commandLength - 4);
	
	if(crcInitSent != crcInitCalc)
	{
		free(frame);
		Throw(CRC_ERROR);
	}

	group = GetUInt16FromBuffer(frame, 4);
	command = GetUInt16FromBuffer(frame, 6);
	
	if(commandLength <= 12)
		MessageDispatcher(group, command, NULL, 0);
	else
		MessageDispatcher(group, command, &frame[8], commandLength - 12);
	
	return true;
}

void MessageDispatcher(uint16_t commandGroup, uint16_t commandCode, uint8_t *dataBuff, uint32_t dataLength)
{
	PrintLnDebugMessage("Received command: ");

	switch(commandGroup)
	{
		case GRP_TEST:
			if(commandCode == CMD_TEST)
			{
				PrintLnDebugMessage("Test");
				HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
			}
			else
				Throw(COMMAND_NOT_VALID_ERROR);
			break;
			
		case GRP_INFO:
			if(commandCode == CMD_GETSERIALNUMBER)
			{
				uint8_t responseFrame[12];
				
				PrintLnDebugMessage("GetSerialNumber");

				// Invio la risposta affermativa
				strcpy((char *)responseFrame, FRAME_HEADER);
				SetBufferFromUInt32(NO_ERROR, responseFrame, 4);
				SetBufferFromUInt32(serialNumber, responseFrame, 8);
				SendToModule_IT(responseFrame, 12);
			}
			else if(commandCode == CMD_GETFIRMWAREVERSION)
			{
				uint8_t responseFrame[12];
				
				PrintLnDebugMessage("GetFirmwareVersion");

				// Invio la risposta affermativa
				strcpy((char *)responseFrame, FRAME_HEADER);
				SetBufferFromUInt32(NO_ERROR, responseFrame, 4);
				SetBufferFromUInt32(firmwareVersion, responseFrame, 8);
				SendToModule_IT(responseFrame, 12);
			}
			else
				Throw(COMMAND_NOT_VALID_ERROR);
			break;
			
		default:
			PrintLnDebugMessage("Unknown group command");
			Throw(GROUP_NOT_VALID_ERROR);
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



