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
#include "crc.h"
#include "debug.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"

/**
  * @brief  Function implementing the bleTask thread.
  * @param  argument: Not used 
  * @retval None
  */
void StartDispatcherTask(void const * argument)
{	
	CEXCEPTION_T ex;

	for(;;)
	{
		mailCommand *commandData = NULL;
		mailCommandResponse *commandResponse = NULL;
		osEvent event;
		
		event = osMailGet(commandMailHandle, osWaitForever);
		commandData = (mailCommand *)event.value.p;
		
		Try
		{
			// Gestione comando
			commandResponse = (mailCommandResponse *)osMailAlloc(commandResponseMailHandle, osWaitForever);
			
			switch(commandData->group)
			{
				case GRP_TEST:
					if(commandData->code == CMD_TEST)
					{
						PrintLnDebugMessage("Test");
						HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
					}
					else
						Throw(COMMAND_NOT_VALID_ERROR);
					break;
					
				case GRP_INFO:
					if(commandData->code == CMD_GETSERIALNUMBER)
					{
						PrintLnDebugMessage("GetSerialNumber");

						// Preparo i dati
						commandResponse->errorCode = NO_ERROR;
						commandResponse->needBuffer = false;
						commandResponse->response = serialNumber;
					}
					else if(commandData->code == CMD_GETFIRMWAREVERSION)
					{						
						PrintLnDebugMessage("GetFirmwareVersion");

						// Preparo i dati
						commandResponse->errorCode = NO_ERROR;
						commandResponse->needBuffer = false;
						commandResponse->response = firmwareVersion;
					}
					else
						Throw(COMMAND_NOT_VALID_ERROR);
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
			commandResponse->needBuffer = false;
			commandResponse->response = 0;		
			
			osMailPut(commandResponseMailHandle, commandResponse);
			osMailFree(commandMailHandle, commandData);
		}
	}
}

