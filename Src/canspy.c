/**
  ******************************************************************************
  * @file           : canspy.c
  * @brief          : CAN Driver for CAN Analyzer
  ******************************************************************************
  */
	
#include "canspy.h"

#include "can.h"

/**
  * @brief  Function implementing the StartCANSpyTask thread.
  * @param  argument: the CAN Line to enable 
  * @retval None
  */
void StartCANSpyTask(void const * argument)
{
	int CANLine = *(int *)argument;

	for(;;)
	{
	}
}

	
	
	
	
	
	