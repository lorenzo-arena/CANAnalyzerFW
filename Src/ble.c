/**
  ******************************************************************************
  * @file           : ble.c
  * @brief          : BLE Driver for AT-09 module
  ******************************************************************************
  */
	
#include "ble.h"

#include "cmsis_os.h"
#include "usart.h"
#include "crc.h"
#include "debug.h"
#include "string.h"
#include "stdlib.h"

/* Private functions */
void InitBLE(void);
void MessageDispatcher(uint8_t *message, int length);
void SendCommandAndReceive(char * message);
void SendToModule_NOIT(char * message, int maxTimeout);
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
		
		HAL_UART_Receive_IT(&huart1, initFrame, initLength);
		while(huart1.RxState != HAL_UART_STATE_READY)
		{
			osDelay(10);
		}
		
		// Loggo la risposta ricevuta
		PrintDebugMessage("Init Frame: ");
		PrintLnDebugBuffer(initFrame, initLength);
		
		if(strncmp((char *)initFrame, "DSCA", 4) == 0)
		{
			uint32_t crcInitCalc = 0;

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
			
			// Aggiungo i 4 byte per il marker
			// e i 4 byte per il Crc32
			lengthDataNext += 8;
			frame = malloc(lengthDataNext);
			if(frame != NULL)
			{
				HAL_UART_Receive_IT(&huart1, frame, lengthDataNext);
				while(huart1.RxState != HAL_UART_STATE_READY)
				{
					osDelay(10);
				}
				
				// Aggiungere controllo sul crc32
				
				// Loggo la risposta ricevuta
				PrintDebugMessage("Frame: ");
				PrintLnDebugBuffer(frame, lengthDataNext);
				
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

#define GRP_TEST 0x3F3F
#define CMD_TEST 0x3F3F

void MessageDispatcher(uint8_t *message, int length)
{
	//if(strcmp((char*)message,"testtest") == 0)
	//{
	//	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	//}
	const int minLength = 12;
	if(length >= minLength)
	{
		if(strncmp((char *)message, "DSCA", 4) == 0)
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

void SendToModule_IT(char * message, int maxTimeout)
{
	// TODO : implementare
	
	//char * toSend;
	//toSend = malloc(strlen(message) + 3);
	//strcpy(toSend, message);
	//strcat(toSend, "\r\n");
	
	//HAL_UART_Transmit(&huart1, (uint8_t*)toSend, strlen(toSend), maxTimeout);

	//free(toSend);
}

void ReceiveFromModule_IT(uint8_t *message, unsigned long maxLength)
{
	HAL_UART_Receive_IT(&huart1, message, maxLength);
	while(huart1.RxState != HAL_UART_STATE_READY)
	{
		osDelay(10);
	}
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



