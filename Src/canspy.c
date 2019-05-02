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

void SetCANLineParameter(int lineNumber, CANSpyParam params)
{
	CAN_HandleTypeDef hcan;
	
	if(lineNumber == 1)
		hcan = hcan1;
	else if(lineNumber == 2)
		hcan = hcan2;
	else
		Throw(PARAMETERS_NOT_CORRECT);
	
	// TODO : creare funzioni che creano i valori dal set di parametri
	// IMPORTANTE: per ora usare una lookup table come fa Capture?
	
	/*
	hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 16;
  hcan1.Init.Mode = CAN_MODE_SILENT;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_1TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
	*/
	
	// ************************** TEMPORANEO
	hcan.Init.Prescaler = 8;
	// ************************** TEMPORANEO
	
	if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }	
}

// NOTE: funzione Capture per la generazione dei parametri CAN
/*
private void GatherCANParameters(out UInt32 registerValue, out UInt32 ID, out UInt32 mask, out bool rilevError, out UInt32 delay)
{
		int timingSel = 0, samplingSel = 0;
		bool otherTiming = false;
		UInt32 bitRate = 0;
		float sampling = 0;

		registerValue = 0;
		ID = 0;
		mask = 0;
		delay = 0;
		rilevError = (checkCANRilevError.IsChecked == true);

		try
		{
				delay = Convert.ToUInt32(editCANDelay.Text);
				if (delay == 0)
						throw new Exception();

				if ((comboCANTiming.SelectedItem != null) && (comboCANTiming.SelectedItem.ToString() == comboCANTiming.Text))
				{
						// Devo comunque prelevare il testo dalla combo poiché se avessi scelto un parametro custom per il sampling
						// ho comunque bisogno di controllarlo
						timingSel = comboCANTiming.SelectedIndex;
						var timingString = new String(comboCANTiming.Text.Where(Char.IsDigit).ToArray());
						bitRate = Convert.ToUInt32(timingString);
				}                   
				else
				{
						var timingString = new String(comboCANTiming.Text.Where(Char.IsDigit).ToArray());
						bitRate = Convert.ToUInt32(timingString);

						if (bitRate == 0 || bitRate > 1000000) // Non posso avere bitrate maggiori di 1M
						{
								throw new Exception();
						}

						otherTiming = true;
				}

				if ((comboCANSamplingPoint.SelectedItem != null) && (comboCANSamplingPoint.SelectedItem.ToString() == comboCANSamplingPoint.Text)) // prelevo il valore impostato del sampling point
				{
						// Devo comunque prelevare il testo dalla combo poiché se avessi scelto un parametro custom per il sampling
						// ho comunque bisogno di controllarlo
						samplingSel = comboCANSamplingPoint.SelectedIndex;
						sampling = Convert.ToSingle(comboCANSamplingPoint.Text);
				} 
				else
				{
						sampling = Convert.ToSingle(comboCANSamplingPoint.Text);

						if (sampling == 0 || sampling < 50)
						{
								throw new Exception();
						}

						otherTiming = true;
				}

				// Controllo con un flag se ho scelto dei parametri "custom"
				if (otherTiming)
				{
						// Qui controllo che i parametri immessi siano uguali a quelli globali settati per ricordare quelli scelti dalla dialog;
						// se supero questo controllo significa che ho settato correttamente i parametri passando dalla dialog di scelta
						if (bitRate != CANTimingPrec || sampling != CANSamplingPointPrec)
						{
								throw new CaptureError(Constants.ERROR_CUSTOMCANPARAMETERS);
						}
						else
								registerValue = CANregisterValuePrec;
				}
				else
				{
						// A questo punto ho sia il bit rate che il sampling point, mi ricavo il vero e proprio timing
						registerValue = _CANTimingArray[timingSel, samplingSel];
				}

				// Ricavo l'ID e la mask
				if (CANExtendedFormat)
				{
						if (CANAcceptAll)
						{
								ID = 0x00000001;
								mask = 0x1FFFFFFF;
						}
						else
						{
								ID = Convert.ToUInt32(editCANID29bit.Text, 16) & 0x1FFFFFFF;
								mask = Convert.ToUInt32(editCANmask29bit.Text, 16) & 0x1FFFFFFF;
						}
						
				}
				else
				{
						if (CANAcceptAll)
						{
								ID = 0x00000001;
								mask = 0x000007FF;
						}
						else
						{
								ID = Convert.ToUInt32(editCANID11bit.Text, 16) & 0x000007FF;
								mask = Convert.ToUInt32(editCANmask11bit.Text, 16) & 0x000007FF;
						}
				}
		}
		catch (Exception ex)
		{
				if (ex is CaptureError)
				{
						throw ex;
				}
				else if (ex is Exception)
				{
						var error = new CaptureError(Constants.ERROR_CUSTOMCANPARAMETERS);
						throw error;
				}
		}
}
				
				*/




