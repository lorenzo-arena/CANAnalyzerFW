/**
  ******************************************************************************
  * @file           : canspy.c
  * @brief          : CAN Driver for CAN Analyzer
  ******************************************************************************
  */
	
#include "canspy.h"

#include "errors.h"
#include "arr_converter.h"
#include "cexception.h"

#include "can.h"

typedef struct
{
	uint32_t bitTiming;
	uint32_t prescaler;
	uint32_t timeSeg1;
	uint32_t timeSeg2;
} CANTimingParamSet;

CANTimingParamSet CANTimingParamLUT[] = {
	{50000, 80, CAN_BS1_7TQ, CAN_BS2_2TQ},
	{100000, 40, CAN_BS1_7TQ, CAN_BS2_2TQ},
	{125000, 32, CAN_BS1_7TQ, CAN_BS2_2TQ},
	{200000, 20, CAN_BS1_7TQ, CAN_BS2_2TQ},
	{250000, 16, CAN_BS1_7TQ, CAN_BS2_2TQ},
	{400000, 10, CAN_BS1_7TQ, CAN_BS2_2TQ},
	{500000, 8, CAN_BS1_7TQ, CAN_BS2_2TQ},
	{1000000, 4, CAN_BS1_7TQ, CAN_BS2_2TQ}
};	

uint32_t CANLine1Interrupts = CAN_IT_RX_FIFO0_MSG_PENDING;
uint32_t CANLine2Interrupts = CAN_IT_RX_FIFO0_MSG_PENDING;

/**
  * @brief  Function implementing the StartCANSpyTask thread.
  * @param  argument: the CAN Line to enable 
  * @retval None
  */
void StartCANSpyTask(void const * argument)
{
	int CANLine = (int)argument;
	
	for(;;)
	{
		CAN_RxHeaderTypeDef   RxHeader;
		uint8_t               RxData[8];
		
		osSignalWait(CANMessageReceivedSignal, osWaitForever);
		HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, RxData);
	}
}

void StartCANLine(int lineNumber)
{
	if(lineNumber == 1)
	{
		if (HAL_CAN_ActivateNotification(&hcan1, CANLine1Interrupts) != HAL_OK)
			Error_Handler();
	}
	else if(lineNumber == 2)
	{
		if (HAL_CAN_ActivateNotification(&hcan2, CANLine2Interrupts) != HAL_OK)
			Error_Handler();
	}
	else
		Throw(FUNCTION_BAD_CALL);
}

void StopCANLine(int lineNumber)
{
	if(lineNumber == 1)
	{
		if (HAL_CAN_DeactivateNotification(&hcan1, CANLine1Interrupts) != HAL_OK)
			Error_Handler();
	}
	else if(lineNumber == 2)
	{
		if (HAL_CAN_DeactivateNotification(&hcan2, CANLine2Interrupts) != HAL_OK)
			Error_Handler();
	}
	else
		Throw(FUNCTION_BAD_CALL);
}

void SetCANLineParameter(int lineNumber, CANSpyParam params)
{
	uint32_t *CANInterrupts;
	uint32_t mask = 0;
	uint32_t id = 0;
	CAN_HandleTypeDef *CANHandler = NULL;
	CANTimingParamSet timingParams;
	CAN_FilterTypeDef filterDef;
	int timingParamsIndex = 0;
	
	if(lineNumber == 1)
	{
		CANHandler = &hcan1;
		CANInterrupts = &CANLine1Interrupts;
	}
	else if(lineNumber == 2)
	{
		CANHandler = &hcan2;
		CANInterrupts = &CANLine2Interrupts;
	}
	else
		Throw(FUNCTION_BAD_CALL);
	
	// Fermo la periferica
	HAL_CAN_Stop(CANHandler);
	
	// Ricavo il set di parametri da utilizzare
	for(timingParamsIndex = 0; timingParamsIndex < sizeof(CANTimingParamLUT)/sizeof(CANTimingParamSet); timingParamsIndex++)
	{
		if(CANTimingParamLUT[timingParamsIndex].bitTiming == params.bitTiming)
			timingParams = CANTimingParamLUT[timingParamsIndex];
	}
	
	// Imposto i parametri per il bitrate richiesto
	CANHandler->Init.Prescaler = timingParams.prescaler;
	CANHandler->Init.TimeSeg1 = timingParams.timeSeg1;
	CANHandler->Init.TimeSeg2 = timingParams.timeSeg2;
	
	if (HAL_CAN_Init(CANHandler) != HAL_OK)
  {
    Error_Handler();
  }
	
	// Imposto gli interrupt da usare per lo start
	if(params.errorReception)
		*CANInterrupts = CAN_IT_RX_FIFO0_MSG_PENDING
										 | CAN_IT_ERROR_WARNING  
										 | CAN_IT_ERROR_PASSIVE  
										 | CAN_IT_BUSOFF         
										 | CAN_IT_LAST_ERROR_CODE
										 | CAN_IT_ERROR          ;
	else
		*CANInterrupts = CAN_IT_RX_FIFO0_MSG_PENDING;

	// Configuro i parametri del filtro
	// Attenzione all'endianness
	if(params.applyMaskAndId)
	{
		mask = params.mask;
		id = params.id;
	}
	else
	{
		if(params.frameFormat == NORMAL_FRAME_FORMAT)
		{
			mask = 0xFFFFF800;
			id = 0;
		}
		else if(params.frameFormat == EXTENDED_FRAME_FORMAT)
		{
			mask = 0xE0000000;
			id = 0;
		}
	}

	filterDef.FilterBank = 0;
	filterDef.SlaveStartFilterBank = 14;
	filterDef.FilterIdHigh = ((id << 5)  | (id >> (32 - 5))) & 0xFFFF; // STID[10:0] & EXTID[17:13]
	filterDef.FilterIdLow = (id >> (11 - 3)) & 0xFFF8; // EXID[12:5] & 3 Reserved bits
	filterDef.FilterMaskIdHigh = ((mask << 5)  | (mask >> (32 - 5))) & 0xFFFF;
	filterDef.FilterMaskIdLow = (mask >> (11 - 3)) & 0xFFF8;
	
	filterDef.FilterMode = CAN_FILTERMODE_IDMASK;
	filterDef.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filterDef.FilterScale = CAN_FILTERSCALE_32BIT;

	filterDef.FilterActivation = CAN_FILTER_ENABLE;
	
	if(HAL_CAN_ConfigFilter(CANHandler, &filterDef) != HAL_OK)
	{
		/* Filter configuration Error */
		Error_Handler();
	}
	
	if (HAL_CAN_Start(CANHandler) != HAL_OK)
	{
		/* Start Error */
		Error_Handler();
	}
}

