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
#include "stdio.h"
#include "fatfs.h"

void DispatchTestCommand(uint16_t command, mailCommand *commandData, mailCommandResponse *responseData);
void DispatchCAN1Command(uint16_t command, mailCommand *commandData, mailCommandResponse *responseData);
void DispatchCAN2Command(uint16_t command, mailCommand *commandData, mailCommandResponse *responseData);
void DispatchInfoCommand(uint16_t command, mailCommand *commandData, mailCommandResponse *responseData);

osThreadId can1TaskHandle = NULL;
osThreadId can2TaskHandle = NULL;

FATFS myFatFS;
FIL myFile;

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
	
	/* init code for FATFS */
  MX_FATFS_Init();

  /* USER CODE BEGIN StartDefaultTask */
	if(f_mount(&myFatFS, "", 1) != FR_OK)
	{
		Error_Handler();
	}

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
		FRESULT searchResult;
		int filesNum = 0;
		
		if(command == CMD_GETCAN1FILESNUM)
		{
			PrintLnDebugMessage(">> Command: Get CAN1 files number");
			searchResult = f_findfirst(&dirInfo, &fileInfo, "\\CAN1", "CAN1*.*");
		}
		else if(command == CMD_GETCAN2FILESNUM)
		{
			PrintLnDebugMessage(">> Command: Get CAN2 files number");
			searchResult = f_findfirst(&dirInfo, &fileInfo, "\\CAN2", "CAN2*.*");
		}
		else if(command == CMD_GETKFILESNUM)
		{
			PrintLnDebugMessage(">> Command: Get K files number");
			searchResult = f_findfirst(&dirInfo, &fileInfo, "\\K", "K*.*");
		}
		
		// Scorro tutti i file per calcolare quanti sono
		while(searchResult == FR_OK && fileInfo.fname[0])
		{
			filesNum++;
			searchResult = f_findnext(&dirInfo, &fileInfo);
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
		FRESULT searchResult;
		int fileIndex = 0;
		int selectedFileIndex = GetUInt32FromBuffer(commandData->dataBuff, 0);
		
		if(command == CMD_GETCAN1FILENAME)
		{
			PrintLnDebugMessage(">> Command: Get CAN1 file name");
			searchResult = f_findfirst(&dirInfo, &fileInfo, "\\CAN1", "CAN1*.*");
		}
		else if(command == CMD_GETCAN2FILENAME)
		{
			PrintLnDebugMessage(">> Command: Get CAN2 file name");
			searchResult = f_findfirst(&dirInfo, &fileInfo, "\\CAN2", "CAN2*.*");
		}
		else if(command == CMD_GETKFILENAME)
		{
			PrintLnDebugMessage(">> Command: Get K file name");
			searchResult = f_findfirst(&dirInfo, &fileInfo, "\\K", "K*.*");
		}
		
		// Scorro tutti i file fino a trovare quello con l'indice corretto
		while(fileIndex != selectedFileIndex && searchResult == FR_OK && fileInfo.fname[0])
		{
			fileIndex++;
			searchResult = f_findnext(&dirInfo, &fileInfo);
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
	else if(command == CMD_GETCAN1FILESIZE ||
		      command == CMD_GETCAN2FILESIZE ||
					command == CMD_GETKFILESIZE)
	{
		FILINFO fileInfo;
		DIR dirInfo;
		FRESULT searchResult;
		int fileIndex = 0;
		int selectedFileIndex = GetUInt32FromBuffer(commandData->dataBuff, 0);
		
		if(command == CMD_GETCAN1FILESIZE)
		{
			PrintLnDebugMessage(">> Command: Get CAN1 file size");
			searchResult = f_findfirst(&dirInfo, &fileInfo, "\\CAN1", "CAN1*.*");
		}
		else if(command == CMD_GETCAN2FILESIZE)
		{
			PrintLnDebugMessage(">> Command: Get CAN2 file size");
			searchResult = f_findfirst(&dirInfo, &fileInfo, "\\CAN2", "CAN2*.*");
		}
		else if(command == CMD_GETKFILESIZE)
		{
			PrintLnDebugMessage(">> Command: Get K file size");
			searchResult = f_findfirst(&dirInfo, &fileInfo, "\\K", "K*.*");
		}
		
		// Scorro tutti i file fino a trovare quello con l'indice corretto
		while(fileIndex != selectedFileIndex && searchResult == FR_OK && fileInfo.fname[0])
		{
			fileIndex++;
			searchResult = f_findnext(&dirInfo, &fileInfo);
		}
		
		f_closedir(&dirInfo);
		
		if(fileIndex == selectedFileIndex && strcmp(fileInfo.fname, "") != 0)
			responseData->response = (uint32_t)fileInfo.fsize;
		else
			Throw(PARAMETERS_NOT_CORRECT);
	}
	else if(command == CMD_GETCAN1FILE ||
					command == CMD_GETCAN2FILE ||
					command == CMD_GETKFILE)
	{
		FIL file;
		char *filePath;

		// Creo la path per il file
		if(command == CMD_GETCAN1FILE)
		{
			PrintLnDebugMessage(">> Command: Download CAN1 file");
			filePath = malloc(6 + strlen((char *)commandData->dataBuff));
			sprintf(filePath, "\\CAN1\\");
			strcat(filePath, (char *)commandData->dataBuff);
		}
		else if(command == CMD_GETCAN2FILE)
		{
			PrintLnDebugMessage(">> Command: Download CAN2 file");
			filePath = malloc(6 + strlen((char *)commandData->dataBuff));
			sprintf(filePath, "\\CAN2\\");
			strcat(filePath, (char *)commandData->dataBuff);
		}
		else if(command == CMD_GETKFILE)
		{
			PrintLnDebugMessage(">> Command: Download K file");
			filePath = malloc(3 + strlen((char *)commandData->dataBuff));
			sprintf(filePath, "\\K\\");
			strcat(filePath, (char *)commandData->dataBuff);
		}
		
		// Apro il file
		if(f_open(&file, filePath, FA_READ) == FR_OK)
		{
			uint32_t sizeRead = 0;
			FSIZE_t fileSize = f_size(&file);
			responseData->response = 0x00000000;
			responseData->responseBuff = malloc(fileSize);
			responseData->responseBuffLength = fileSize;
			
			f_read(&file, responseData->responseBuff, fileSize, &sizeRead);
			free(filePath);
		}
		else
		{
			free(filePath);
			Throw(PARAMETERS_NOT_CORRECT);
		}
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

