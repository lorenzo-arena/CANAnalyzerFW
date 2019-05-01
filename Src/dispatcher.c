/**
  ******************************************************************************
  * @file           : dispatcher.c
  * @brief          : Message dispatcher for CAN Analyzer
  ******************************************************************************
  */
	
#include "dispatcher.h"

#include "errors.h"
#include "commands.h"
#include "cexception.h"
#include "mailformats.h"
#include "canspy.h"
#include "crc.h"
#include "debug.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"

uint32_t DispatchTestCommand(uint16_t command, uint8_t *responseBuff, uint32_t *responseLength);
uint32_t DispatchCAN1Command(uint16_t command, uint8_t *responseBuff, uint32_t *responseLength);
uint32_t DispatchCAN2Command(uint16_t command, uint8_t *responseBuff, uint32_t *responseLength);
uint32_t DispatchInfoCommand(uint16_t command, uint8_t *responseBuff, uint32_t *responseLength);

osThreadId can1TaskHandle = NULL;
osThreadId can2TaskHandle = NULL;

/**
  * @brief  Function implementing the StartDispatcherTask thread.
  * @param  argument: Not used 
  * @retval None
  */
void StartDispatcherTask(void const * argument)
{	
	CEXCEPTION_T ex;
	mailCommand *commandData = NULL;
	mailCommandResponse *commandResponse = NULL;
	osEvent event;

	for(;;)
	{		
		event = osMailGet(commandMailHandle, osWaitForever);
		commandData = (mailCommand *)event.value.p;
		
		Try
		{
			// Gestione comando
			commandResponse = (mailCommandResponse *)osMailAlloc(commandResponseMailHandle, osWaitForever);
			commandResponse->errorCode = NO_ERROR;
			commandResponse->response = 0;
			commandResponse->responseBuff = NULL;
			commandResponse->responseBuffLength = 0;
			
			switch(commandData->group)
			{
				case GRP_TEST:
					commandResponse->response = DispatchTestCommand(commandData->code, commandResponse->responseBuff, &(commandResponse->responseBuffLength));
					break;
					
				case GRP_INFO:
					commandResponse->response = DispatchInfoCommand(commandData->code, commandResponse->responseBuff, &(commandResponse->responseBuffLength));
					break;
				
				case GRP_CAN_LINE1:
					commandResponse->response = DispatchCAN1Command(commandData->code, commandResponse->responseBuff, &(commandResponse->responseBuffLength));
					break;
				
				case GRP_CAN_LINE2:
					commandResponse->response = DispatchCAN2Command(commandData->code, commandResponse->responseBuff, &(commandResponse->responseBuffLength));
					break;
					
				default:
					PrintLnDebugMessage("Unknown group command");
					Throw(GROUP_NOT_VALID_ERROR);
			}

			osMailPut(commandResponseMailHandle, commandResponse);
			osMailFree(commandMailHandle, commandData);
		}
		Catch(ex)
		{
			// Preparo i dati
			commandResponse->errorCode = ex;	
			
			osMailPut(commandResponseMailHandle, commandResponse);
			osMailFree(commandMailHandle, commandData);
		}
	}
}

uint32_t DispatchTestCommand(uint16_t command, uint8_t *responseBuff, uint32_t *responseLength)
{
	if(command == CMD_TEST)
	{
		PrintLnDebugMessage("Test");
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	}
	else
		Throw(COMMAND_NOT_VALID_ERROR);
	
	return 0;
}

uint32_t DispatchInfoCommand(uint16_t command, uint8_t *responseBuff, uint32_t *responseLength)
{
	if(command == CMD_GETSERIALNUMBER)
	{
		PrintLnDebugMessage("GetSerialNumber");

		// Preparo i dati
		return serialNumber;
	}
	else if(command == CMD_GETFIRMWAREVERSION)
	{						
		PrintLnDebugMessage("GetFirmwareVersion");

		// Preparo i dati
		return firmwareVersion;
	}
	else
		Throw(COMMAND_NOT_VALID_ERROR);
	
	return 0;
}

uint32_t DispatchCAN1Command(uint16_t command, uint8_t *responseBuff, uint32_t *responseLength)
{
	if(command == CMD_STARTCANLINE)
	{
		int lineCode = 1;
	
		// Controllo se il thread e' gia' stato avviato
		if(can1TaskHandle != NULL)
			osThreadTerminate(can1TaskHandle);
		
		// Avvio un nuovo thread CAN		
		osThreadDef(can1Task, StartCANSpyTask, osPriorityNormal, 0, 128);	
		can1TaskHandle = osThreadCreate(osThread(can1Task), &lineCode);
	}
	else if(command == CMD_STOPCANLINE)
	{		
		osStatus state = osThreadTerminate(can1TaskHandle);
		
		if(state != osOK)
			Throw(COMMAND_NOT_EXECUTED_ERROR);
	}
	else if(command == CMD_SETCANPARAM)
	{
		// TODO : implementare
	}
	else
		Throw(COMMAND_NOT_VALID_ERROR);
	
	return 0;
}

uint32_t DispatchCAN2Command(uint16_t command, uint8_t *responseBuff, uint32_t *responseLength)
{
	if(command == CMD_STARTCANLINE)
	{
		int lineCode = 1;
	
		// Controllo se il thread e' gia' stato avviato
		if(can2TaskHandle != NULL)
			osThreadTerminate(can2TaskHandle);
		
		// Avvio un nuovo thread CAN		
		osThreadDef(can2Task, StartCANSpyTask, osPriorityNormal, 0, 128);	
		can2TaskHandle = osThreadCreate(osThread(can2Task), &lineCode);
	}
	else if(command == CMD_STOPCANLINE)
	{
		osStatus state = osThreadTerminate(can2TaskHandle);
		
		if(state != osOK)
			Throw(COMMAND_NOT_EXECUTED_ERROR);
	}
	else if(command == CMD_SETCANPARAM)
	{
		// TODO : implementare
	}
	else
		Throw(COMMAND_NOT_VALID_ERROR);
	
	return 0;
}

