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
		if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
			Error_Handler();
	}
	else if(lineNumber == 2)
	{
		if (HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
			Error_Handler();
	}
	else
		Throw(FUNCTION_BAD_CALL);
}

void StopCANLine(int lineNumber)
{
	if(lineNumber == 1)
	{
		if (HAL_CAN_DeactivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
			Error_Handler();
	}
	else if(lineNumber == 2)
	{
		if (HAL_CAN_DeactivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
			Error_Handler();
	}
	else
		Throw(FUNCTION_BAD_CALL);
}

void SetCANLineParameter(int lineNumber, CANSpyParam params)
{
	CAN_HandleTypeDef *CANHandler = NULL;
	CANTimingParamSet timingParams;
	CAN_FilterTypeDef filterDef;
	int timingParamsIndex = 0;
	
	if(lineNumber == 1)
		CANHandler = &hcan1;
	else if(lineNumber == 2)
		CANHandler = &hcan2;
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

	// Configuro i parametri del filtro
	// Attenzione all'endianness
	filterDef.FilterBank = 0;
	filterDef.SlaveStartFilterBank = 14;
	filterDef.FilterIdHigh = ((params.id & 0xFFFF0000) >> 16);
	filterDef.FilterIdLow = (params.id & 0x0000FFFF);
	filterDef.FilterMaskIdHigh = ((params.mask & 0xFFFF0000) >> 16);
	filterDef.FilterMaskIdLow = (params.mask & 0x0000FFFF);
	
	filterDef.FilterMode = CAN_FILTERMODE_IDMASK;
	filterDef.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filterDef.FilterScale = CAN_FILTERSCALE_32BIT;
	
	//if(params.applyMaskAndId)
		filterDef.FilterActivation = CAN_FILTER_ENABLE;
	//else
	//	filterDef.FilterActivation = CAN_FILTER_DISABLE;
	
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

