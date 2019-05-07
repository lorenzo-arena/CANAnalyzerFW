/**
  ******************************************************************************
  * @file           : dispatcher.c
  * @brief          : Message dispatcher for CAN Analyzer
  ******************************************************************************
  */
	
#include "dispatcher.h"

#include "errors.h"
#include "commands.h"
#include "arr_converter.h"
#include "cexception.h"
#include "mailformats.h"
#include "canspy.h"
#include "crc.h"
#include "debug.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"

void DispatchTestCommand(uint16_t command, mailCommand *commandData, mailCommandResponse *responseData);
void DispatchCAN1Command(uint16_t command, mailCommand *commandData, mailCommandResponse *responseData);
void DispatchCAN2Command(uint16_t command, mailCommand *commandData, mailCommandResponse *responseData);
void DispatchInfoCommand(uint16_t command, mailCommand *commandData, mailCommandResponse *responseData);

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
					DispatchTestCommand(commandData->code, commandData, commandResponse);
					break;
					
				case GRP_INFO:
					DispatchInfoCommand(commandData->code, commandData, commandResponse);
					break;
				
				case GRP_CAN_LINE1:
					DispatchCAN1Command(commandData->code, commandData, commandResponse);
					break;
				
				case GRP_CAN_LINE2:
					DispatchCAN2Command(commandData->code, commandData, commandResponse);
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

void DispatchTestCommand(uint16_t command, mailCommand *commandData, mailCommandResponse *responseData)
{
	if(command == CMD_TEST)
	{
		PrintLnDebugMessage("Test");
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	}
	else
		Throw(COMMAND_NOT_VALID_ERROR);
}

void DispatchInfoCommand(uint16_t command, mailCommand *commandData, mailCommandResponse *responseData)
{
	if(command == CMD_GETSERIALNUMBER)
	{
		PrintLnDebugMessage("GetSerialNumber");

		// Preparo i dati
		responseData->response = serialNumber;
	}
	else if(command == CMD_GETFIRMWAREVERSION)
	{						
		PrintLnDebugMessage("GetFirmwareVersion");

		// Preparo i dati
		responseData->response = firmwareVersion;
	}
	else
		Throw(COMMAND_NOT_VALID_ERROR);
}

void DispatchCAN1Command(uint16_t command, mailCommand *commandData, mailCommandResponse *responseData)
{
	if(command == CMD_STARTCANLINE)
	{
		StartCANLine(1);
	}
	else if(command == CMD_STOPCANLINE)
	{		
		StopCANLine(1);
	}
	else if(command == CMD_SETCANPARAM)
	{
		CANSpyParam params;
		params.bitTiming = GetUInt32FromBuffer(commandData->dataBuff, 0);
		params.frameFormat = GetUInt32FromBuffer(commandData->dataBuff, 4);
		params.errorReception = GetUInt32FromBuffer(commandData->dataBuff, 8) != 0;
		params.applyMaskAndId = GetUInt32FromBuffer(commandData->dataBuff, 12) != 0;
		params.mask = GetUInt32FromBuffer(commandData->dataBuff, 16);
		params.id = GetUInt32FromBuffer(commandData->dataBuff, 20);

		SetCANLineParameter(1, params);
	}
	else
		Throw(COMMAND_NOT_VALID_ERROR);
}

void DispatchCAN2Command(uint16_t command, mailCommand *commandData, mailCommandResponse *responseData)
{
	if(command == CMD_STARTCANLINE)
	{
		StartCANLine(2);
	}
	else if(command == CMD_STOPCANLINE)
	{		
		StopCANLine(2);
	}
	else if(command == CMD_SETCANPARAM)
	{
		CANSpyParam params;
		params.bitTiming = GetUInt32FromBuffer(commandData->dataBuff, 0);
		params.frameFormat = GetUInt32FromBuffer(commandData->dataBuff, 4);
		params.errorReception = GetUInt32FromBuffer(commandData->dataBuff, 8) != 0;
		params.applyMaskAndId = GetUInt32FromBuffer(commandData->dataBuff, 12) != 0;
		params.mask = GetUInt32FromBuffer(commandData->dataBuff, 16);
		params.id = GetUInt32FromBuffer(commandData->dataBuff, 20);

		SetCANLineParameter(2, params);
	}
	else
		Throw(COMMAND_NOT_VALID_ERROR);
}

