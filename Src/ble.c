/**
  ******************************************************************************
  * @file           : ble.c
  * @brief          : BLE Driver for AT-09 module
  ******************************************************************************
  */
	
#include "ble.h"
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"
#include "debug.h"
#include "string.h"
#include "stdlib.h"

/* Private functions */
void InitBLE(void);
void MessageDispatcher(uint8_t *message, int length);
void SendCommandAndReceive(char * message);
void SendToModule_NOIT(char * message, int maxTimeout);
void ReceiveFromModule_NOIT(char * message, unsigned long maxLength, unsigned int maxTimout);
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
		const int initLength = 10;
		uint8_t initFrame[initLength];
		
		HAL_UART_Receive_IT(&huart1, initFrame, initLength);
		while(huart1.RxState != HAL_UART_STATE_READY)
		{
			osDelay(10);
		}
		
		// Loggo la risposta ricevuta
		PrintDebugMessage("Init Frame: ");
		PrintLnDebugMessage((char *)initFrame);
		
		if(strncmp((char *)initFrame, "CA", 2) == 0)
		{
			uint16_t crcInit = (initFrame[8] << 8) |
								(initFrame[9]);
							
			uint16_t crcNext = (initFrame[6] << 8) |
								(initFrame[7]);
								
			uint32_t lengthNext = (initFrame[2] << 24) |
								  (initFrame[3] << 16) |
								  (initFrame[4] << 8)  |
								  (initFrame[5]);
								  
			uint8_t *frame = NULL;
			
			frame = malloc(lengthNext);
			
			if(frame != NULL)
			{
				HAL_UART_Receive_IT(&huart1, frame, lengthNext);
				while(huart1.RxState != HAL_UART_STATE_READY)
				{
					osDelay(10);
				}
				
				// Aggiungere controllo sul crc32
				
				// Loggo la risposta ricevuta
				PrintDebugMessage("Frame: ");
				PrintLnDebugMessage((char *)frame);
				
				MessageDispatcher(frame, lengthNext);
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



