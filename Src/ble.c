/**
  ******************************************************************************
  * @file           : ble.c
  * @brief          : BLE Driver for AT-09 module
  ******************************************************************************
  */
	
#include "ble.h"

#include "commands.h"
#include "usart.h"
#include "crc.h"
#include "debug.h"
#include "string.h"
#include "stdlib.h"

#define FRAME_HEADER "DSCA"

/* Private functions */
void InitBLE(void);
void MessageDispatcher(uint8_t *message, int length);
void SendCommandAndReceive(char * message);
void SendToModule_NOIT(char * message, int maxTimeout);
void SendToModule_IT(uint8_t *message, unsigned long maxLength);
void ReceiveFromModule_NOIT(char * message, unsigned long maxLength, unsigned int maxTimout);
void ReceiveFromModule_IT(uint8_t *message, unsigned long maxLength);
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
		// TODO : creare thread generico per la gestione dei messaggi
		const int initLength = 12;
		uint8_t initFrame[initLength];
		const int initResponseLength = 8;
		uint8_t initResponseFrame[initResponseLength];
		
		PrintLnDebugMessage("Init Frame: ");
		ReceiveFromModule_IT(initFrame, initLength);
		
		if(strncmp((char *)initFrame, FRAME_HEADER, 4) == 0)
		{
			uint32_t crcInitCalc = 0;
			uint32_t errorCode = 0x00000000;

			uint32_t lengthDataNext = (initFrame[4] << 24) |
						(initFrame[5] << 16) |
						(initFrame[6] << 8)  |
						(initFrame[7]);
							
			uint32_t crcInit = (initFrame[8] << 24) |
						(initFrame[9] << 16) |
						(initFrame[10] << 8)  |
						(initFrame[11]);
								  
			uint8_t *frame = NULL;
			
			// Controllo il CRC del frame di init, escludendo i byte del CRC
			crcInitCalc = CRC32_Compute(initFrame, initLength - 4);
			
			if(crcInit == crcInitCalc)
				PrintLnDebugMessage("Crc32 OK!");
			
			// Invio la risposta affermativa
			strcpy((char *)initResponseFrame, FRAME_HEADER);
			memcpy(initResponseFrame + 4, &errorCode, sizeof(uint32_t));
			
			SendToModule_IT(initResponseFrame, initResponseLength);
			
			// Aggiungo i 4 byte per il marker
			// e i 4 byte per il Crc32
			lengthDataNext += 8;
			frame = malloc(lengthDataNext);
			if(frame != NULL)
			{
				PrintLnDebugMessage("Frame: ");
				ReceiveFromModule_IT(frame, lengthDataNext);
				
				// TODO : Aggiungere controllo sul crc32
				
				MessageDispatcher(frame, lengthDataNext);
			}
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

void MessageDispatcher(uint8_t *message, int length)
{
	//if(strcmp((char*)message,"testtest") == 0)
	//{
	//	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	//}
	const int minLength = 12;
	if(length >= minLength)
	{
		if(strncmp((char *)message, FRAME_HEADER, 4) == 0)
		{
			uint16_t group = *((uint16_t *)&message[4]);
			uint16_t command = *((uint16_t *)&message[6]);
			
			switch(group)
			{
				case GRP_TEST:
					if(command == CMD_TEST)
					{
						HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
					}
					break;
			}
		}
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



