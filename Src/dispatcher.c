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
#include "fatfs.h"

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
	if(command == CMD_BLINK)
	{
		PrintLnDebugMessage(">> Command: Blink");
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	}
	else if(command == CMD_SLEEP)
	{
		PrintLnDebugMessage(">> Command: Sleep");
		PrintLnDebugMessage("Sleeping...");
		osDelay(5000);
		PrintLnDebugMessage("Resumed");
	}
	else
		Throw(COMMAND_NOT_VALID_ERROR);
}

void DispatchInfoCommand(uint16_t command, mailCommand *commandData, mailCommandResponse *responseData)
{
	if(command == CMD_GETSERIALNUMBER)
	{
		PrintLnDebugMessage(">> Command: GetSerialNumber");

		// Preparo i dati
		responseData->response = serialNumber;
	}
	else if(command == CMD_GETFIRMWAREVERSION)
	{						
		PrintLnDebugMessage(">> Command: GetFirmwareVersion");

		// Preparo i dati
		responseData->response = firmwareVersion;
	}
	else if(command == CMD_GETCAN1FILESNUM ||
		      command == CMD_GETCAN2FILESNUM ||
					command == CMD_GETKFILESNUM)
	{
		FILINFO fileInfo;
		DIR dirInfo;
		int filesNum = 0;
		
		if(command == CMD_GETCAN1FILESNUM)
		{
			PrintLnDebugMessage(">> Command: Get CAN1 files number");
			f_opendir(&dirInfo, "\\CAN1");
		}
		else if(command == CMD_GETCAN2FILESNUM)
		{
			PrintLnDebugMessage(">> Command: Get CAN2 files number");
			f_opendir(&dirInfo, "\\CAN2");
		}
		else if(command == CMD_GETKFILESNUM)
		{
			PrintLnDebugMessage(">> Command: Get K files number");
			f_opendir(&dirInfo, "\\K");
		}
		
		f_readdir(&dirInfo, &fileInfo);
		while(strcmp(fileInfo.fname, "") != 0)
		{
			filesNum++;
			f_readdir(&dirInfo, &fileInfo);
		}
		f_closedir(&dirInfo);

		responseData->response = filesNum;
	}
	else if(command == CMD_GETCAN1FILENAME ||
		      command == CMD_GETCAN2FILENAME ||
					command == CMD_GETKFILENAME)
	{
		FILINFO fileInfo;
		DIR dirInfo;
		int fileIndex = 0;
		int selectedFileIndex = GetUInt32FromBuffer(commandData->dataBuff, 0);
		
		if(command == CMD_GETCAN1FILENAME)
		{
			PrintLnDebugMessage(">> Command: Get CAN1 file name");
			f_opendir(&dirInfo, "\\CAN1");
		}
		else if(command == CMD_GETCAN2FILENAME)
		{
			PrintLnDebugMessage(">> Command: Get CAN2 file name");
			f_opendir(&dirInfo, "\\CAN2");
		}
		else if(command == CMD_GETKFILENAME)
		{
			PrintLnDebugMessage(">> Command: Get K file name");
			f_opendir(&dirInfo, "\\K");
		}
		
		f_readdir(&dirInfo, &fileInfo);
		while(fileIndex != selectedFileIndex && strcmp(fileInfo.fname, "") != 0)
		{
			fileIndex++;
			f_readdir(&dirInfo, &fileInfo);
		}
		f_closedir(&dirInfo);
		
		if(fileIndex == selectedFileIndex && strcmp(fileInfo.fname, "") != 0)
		{
			responseData->response = 0x00000000;
			responseData->responseBuff = malloc(strlen(fileInfo.fname));
			responseData->responseBuffLength = strlen(fileInfo.fname);
			memcpy(responseData->responseBuff, fileInfo.fname, strlen(fileInfo.fname));
		}
		else
			Throw(PARAMETERS_NOT_CORRECT);
	}
	else
		Throw(COMMAND_NOT_VALID_ERROR);
}

void DispatchCAN1Command(uint16_t command, mailCommand *commandData, mailCommandResponse *responseData)
{
	if(command == CMD_STARTCANLINE)
	{
		PrintLnDebugMessage(">> Command: Start CAN1 Line");
		StartCANLine(1);
	}
	else if(command == CMD_STOPCANLINE)
	{	
		PrintLnDebugMessage(">> Command: Stop CAN1 Line");		
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

		PrintLnDebugMessage(">> Command: Set CAN1 parameters");
		SetCANLineParameter(1, params);
	}
	else
		Throw(COMMAND_NOT_VALID_ERROR);
}

void DispatchCAN2Command(uint16_t command, mailCommand *commandData, mailCommandResponse *responseData)
{
	if(command == CMD_STARTCANLINE)
	{
		PrintLnDebugMessage(">> Command: Start CAN2 Line");
		StartCANLine(2);
	}
	else if(command == CMD_STOPCANLINE)
	{
		PrintLnDebugMessage(">> Command: Stop CAN1 Line");
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

		PrintLnDebugMessage(">> Command: Set CAN2 parameters");
		SetCANLineParameter(2, params);
	}
	else
		Throw(COMMAND_NOT_VALID_ERROR);
}

