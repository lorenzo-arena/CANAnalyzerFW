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
		Try
		{

		}
		Catch(ex)
		{
			
		}
	}
}

