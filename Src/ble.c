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
#include "mailformats.h"
#include "usart.h"
#include "can.h"
#include "crc.h"
#include "debug.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"

#define FRAME_HEADER "DSCA"

// Per adesso cambio
//const int maxCommandTimeout = 500;
const int maxCommandTimeout = osWaitForever;

/* Private functions */
void InitBLE(void);
void CallDispatcher(uint16_t commandGroup, uint16_t commandCode, uint8_t *dataBuff, uint32_t dataLength);
void SendCommandAndReceive(char * message);
void SendError(uint32_t errorCode);
void SendToModule_NOIT(char * message, int maxTimeout);
void SendToModule_IT(uint8_t *message, uint32_t maxLength);
void ReceiveFromModule_NOIT(char * message, uint32_t maxLength, uint32_t maxTimeout);
void ReceiveFromModule_IT(uint8_t *message, uint32_t maxLength, uint32_t timeout);
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
			if(ex == TIMEOUT_ERROR)
				PrintLnDebugMessage("Timeout!");
			else
			{
				// Invio il messaggio di errore
				SendError(ex);
			}
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
	const int initResponseLength = 12;
	uint8_t initResponseFrame[initResponseLength];
	uint32_t crcInitCalc = 0;
	uint32_t lengthDataNext = 0;
	uint32_t crcInitSent = 0;
	uint32_t crcResponse = 0;

	PrintLnDebugMessage("Receiving Init Frame..");
	ReceiveFromModule_IT(initFrame, initLength, osWaitForever);
	
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
	
	// Calcolo il crc
	crcResponse = CRC32_Compute(initResponseFrame, sizeof(initResponseFrame) - 4);
	SetBufferFromUInt32(crcResponse, initResponseFrame, sizeof(initResponseFrame) - 4);
	
	SendToModule_IT(initResponseFrame, initResponseLength);		
	
	return true;
}

bool ReceivePacketOKCommand()
{
	CEXCEPTION_T ex;
	uint32_t commandLength = 0;
	uint8_t *frame = NULL;
	uint32_t crcInitSent = 0;
	uint32_t crcInitCalc = 0;
	uint32_t packetCommand = 0;

	Try
	{
		// Aggiungo i 4 byte per il marker
		// e i 4 byte per il Crc32
		commandLength = 12;
		frame = malloc(commandLength);
		if(frame == NULL)
			Throw(MEMORY_ERROR);
	
		PrintLnDebugMessage("Receiving PacketOK Frame..");
		ReceiveFromModule_IT(frame, commandLength, maxCommandTimeout);
		
		if(strncmp((char *)frame, FRAME_HEADER, 4) != 0)
			Throw(MARKER_ERROR);

		crcInitSent = GetUInt32FromBuffer(frame, commandLength - 4);
		
		// Controllo il CRC del frame di init, escludendo i byte del CRC
		crcInitCalc = CRC32_Compute(frame, commandLength - 4);
		
		if(crcInitSent != crcInitCalc)
			Throw(CRC_ERROR);

		packetCommand = GetUInt32FromBuffer(frame, 4);
		
		if(packetCommand != 0x3F3F3F3F)
			Throw(COMMAND_NOT_VALID_ERROR);			
	}
	Catch(ex)
	{
		if(frame != NULL)
			free(frame);
		Throw(ex);
	}
	
	free(frame);
	return true;
}

bool ReceiveCommand(uint32_t length)
{
	CEXCEPTION_T ex;
	uint32_t commandLength = length;
	uint8_t *frame = NULL;
	uint32_t crcInitSent = 0;
	uint32_t crcInitCalc = 0;
	uint16_t group = 0;
	uint16_t command = 0;

	Try
	{
		// Aggiungo i 4 byte per il marker
		// e i 4 byte per il Crc32
		commandLength += 8;
		frame = malloc(commandLength);
		if(frame == NULL)
			Throw(MEMORY_ERROR);
	
		PrintLnDebugMessage("Receiving Command Frame..");
		ReceiveFromModule_IT(frame, commandLength, maxCommandTimeout);
		
		if(strncmp((char *)frame, FRAME_HEADER, 4) != 0)
			Throw(MARKER_ERROR);

		crcInitSent = GetUInt32FromBuffer(frame, commandLength - 4);
		
		// Controllo il CRC del frame di init, escludendo i byte del CRC
		crcInitCalc = CRC32_Compute(frame, commandLength - 4);
		
		if(crcInitSent != crcInitCalc)
			Throw(CRC_ERROR);

		group = GetUInt16FromBuffer(frame, 4);
		command = GetUInt16FromBuffer(frame, 6);
		
		if(commandLength <= 12)
			CallDispatcher(group, command, NULL, 0);
		else
			CallDispatcher(group, command, &frame[8], commandLength - 12);
	}
	Catch(ex)
	{
		if(frame != NULL)
			free(frame);
		Throw(ex);
	}
	
	free(frame);
	return true;
}

void CallDispatcher(uint16_t commandGroup, uint16_t commandCode, uint8_t *dataBuff, uint32_t dataLength)
{
	CEXCEPTION_T ex;
	mailCommand *commandData = NULL;
	mailCommandResponse *commandResponse = NULL;
	osEvent event;
	uint8_t *responseFrame = NULL;
	uint32_t crcResponse = 0;
	
	Try
	{
		// Alloco lo spazio per la mail contenente i dati del comando
		commandData = (mailCommand *)osMailAlloc(commandMailHandle, osWaitForever);
		
		// Imposto i parametri del comando ricevuto
		commandData->group = commandGroup;
		commandData->code = commandCode;
		commandData->dataBuff = dataBuff;
		commandData->dataLength = dataLength;
		
		// Invio i parametri al Dispatcher
		osMailPut(commandMailHandle, commandData);
	
		// Mi metto in attesa di una risposta dal Dispatcher,
		// tenendo attiva la comunicazione con la app tramite il comando CMD_WAIT
		event = osMailGet(commandResponseMailHandle, 50);
		while(event.status == osEventTimeout)
		{
			responseFrame = malloc(16);
			strcpy((char *)responseFrame, FRAME_HEADER);
			SetBufferFromUInt32(NO_ERROR, responseFrame, 4);
			SetBufferFromUInt32(((uint32_t)GRP_UTILITY << 16) | CMD_WAIT, responseFrame, 8);
			
			// Calcolo il crc
			crcResponse = CRC32_Compute(responseFrame, 16 - 4);
			SetBufferFromUInt32(crcResponse, responseFrame, 16 - 4);
			
			// Invio la risposta affermativa
			SendToModule_IT(responseFrame, 16);
			free(responseFrame);
			
			event = osMailGet(commandResponseMailHandle, 50);
		}
		
		commandResponse = (mailCommandResponse *)event.value.p;       // ".p" indicates that the message is a pointer
		
		// Gestione risposta
		if(commandResponse->errorCode != 0)
		{
			osMailFree(commandResponseMailHandle, commandResponse);
			Throw(commandResponse->errorCode);
		}

		if(commandResponse->responseBuff == NULL)
		{
			const int messageLength = 16;
			responseFrame = malloc(messageLength);
			strcpy((char *)responseFrame, FRAME_HEADER);
			SetBufferFromUInt32(NO_ERROR, responseFrame, 4);
			SetBufferFromUInt32(commandResponse->response, responseFrame, 8);
			
			// Calcolo il crc
			crcResponse = CRC32_Compute(responseFrame, messageLength - 4);
			SetBufferFromUInt32(crcResponse, responseFrame, messageLength - 4);
			
			// Invio la risposta affermativa
			SendToModule_IT(responseFrame, messageLength);
			free(responseFrame);
		}
		else
		{
			const int messageLength = 16 + commandResponse->responseBuffLength;
			responseFrame = malloc(messageLength);
			strcpy((char *)responseFrame, FRAME_HEADER);
			SetBufferFromUInt32(NO_ERROR, responseFrame, 4);
			SetBufferFromUInt32(commandResponse->response, responseFrame, 8);
			
			memcpy(responseFrame + 12, commandResponse->responseBuff, commandResponse->responseBuffLength);
			free(commandResponse->responseBuff);
			
			if(messageLength <= 20)
			{
				// Calcolo il crc
				crcResponse = CRC32_Compute(responseFrame, messageLength - 4);
				SetBufferFromUInt32(crcResponse, responseFrame, messageLength - 4);
				
				// Invio la risposta affermativa
				SendToModule_IT(responseFrame, messageLength);
			}
			else
			{
				// Invio la risposta a pacchetti di 20 byte
				int packetsIndex = 0;
				int packetsNum = ((messageLength % 20) == 0) ? (messageLength / 20) : ((messageLength / 20) + 1);
				const int packetCommandLength = 20;
				uint8_t *packetCommandFrame = malloc(packetCommandLength);
				
				PrintLnDebugMessage("*** Start packets transmission ***");
				
				strcpy((char *)packetCommandFrame, FRAME_HEADER);
				SetBufferFromUInt32(NO_ERROR, packetCommandFrame, 4);
				SetBufferFromUInt32(0x3F3F3F3F, packetCommandFrame, 8);
				
				// Imposto la size che il dispositivo dovra' ricevere
				SetBufferFromUInt32(messageLength, packetCommandFrame, 12);
				
				// Calcolo il crc
				crcResponse = CRC32_Compute(packetCommandFrame, packetCommandLength - 4);
				SetBufferFromUInt32(crcResponse, packetCommandFrame, packetCommandLength - 4);
				
				// Invio la risposta per l'invio dei pacchetti
				SendToModule_IT(packetCommandFrame, packetCommandLength);
				free(packetCommandFrame);
				
				ReceivePacketOKCommand();
				
				for(packetsIndex = 0; packetsIndex < packetsNum; packetsIndex++)
				{
					uint32_t packetSize = (packetsIndex < packetsNum - 1) ? 20 : (messageLength % 20);
					uint8_t *packetFrame = malloc(packetSize);
					
					memcpy(packetFrame, responseFrame + 20 * packetsIndex, packetSize);
					
					// Invio il pacchetto corrente
					SendToModule_IT(packetFrame, packetSize);
					free(packetFrame);
					
					ReceivePacketOKCommand();
				}
			}
			
			free(responseFrame);
		}

		osMailFree(commandResponseMailHandle, commandResponse);
	}
	Catch(ex)
	{
		if(responseFrame != NULL)
			free(responseFrame);
		
		Throw(ex);
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

void SendError(uint32_t errorCode)
{
	uint8_t errorFrame[12];
	uint32_t crcResponse = 0;
				
	// Imposto l'header
	strcpy((char *)errorFrame, FRAME_HEADER);
	
	// Imposto il codice di errore
	memcpy(errorFrame + 4, &errorCode, sizeof(uint32_t));
	
	// Calcolo il crc
	crcResponse = CRC32_Compute(errorFrame, sizeof(errorFrame) - 4);
	SetBufferFromUInt32(crcResponse, errorFrame, sizeof(errorFrame) - 4);

	SendToModule_IT(errorFrame, sizeof(errorFrame));
}

void SendToModule_IT(uint8_t *message, uint32_t maxLength)
{
	// Loggo il messaggio inviato
	PrintDebugMessage("Sending: ");
	PrintLnDebugBuffer(message, maxLength);
	
	if(message[0] == 0x00 || message[0] == 0xFF)
		Throw(NO_ERROR);
	
	HAL_UART_Transmit_IT(&huart1, message, maxLength);
	osSignalWait(UART1MessageSentSignal, osWaitForever);
}

void ReceiveFromModule_IT(uint8_t *message, uint32_t maxLength, uint32_t timeout)
{
	HAL_UART_Receive_IT(&huart1, message, maxLength);
	osSignalWait(UART1MessageReceivedSignal, timeout);
	
	if(huart1.RxState == HAL_UART_STATE_BUSY_RX)
		Throw(TIMEOUT_ERROR);
	
	// Loggo la risposta ricevuta
	PrintDebugMessage("Received: ");
	PrintLnDebugBuffer(message, maxLength);
}

void SendToModule_NOIT(char * message, int maxTimeout)
{
	char * toSend;
	
	toSend = malloc(strlen(message) + 3);
	if(toSend == NULL)
		Throw(MEMORY_ERROR);
	
	strcpy(toSend, message);
	strcat(toSend, "\r\n");
	
	HAL_UART_Transmit(&huart1, (uint8_t*)toSend, strlen(toSend), maxTimeout);

	free(toSend);
}

void ReceiveFromModule_NOIT(char * message, uint32_t maxLength, uint32_t maxTimeout)
{
	HAL_UART_Receive(&huart1, (uint8_t*)message, maxLength, maxTimeout);
}



