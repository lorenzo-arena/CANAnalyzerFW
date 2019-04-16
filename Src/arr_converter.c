/**
  ******************************************************************************
  * @file           : arr_converter.c
  * @brief          : Functions to convert byte array to uint
  ******************************************************************************
  */
	
#include "arr_converter.h"

uint16_t GetUInt16FromBuffer(uint8_t *buff, uint32_t index)
{
	return (buff[index] << 8) |
				 (buff[index + 1]);
}

uint32_t GetUInt32FromBuffer(uint8_t *buff, uint32_t index)
{
	return (buff[index] << 24) |
				 (buff[index + 1] << 16) |
				 (buff[index + 2] << 8) |
				 (buff[index + 3]);
}

void SetBufferFromUInt16(uint16_t toSet, uint8_t *buff, uint32_t index)
{
	buff[index] = (toSet & 0xFF00) >> 8;
	buff[index + 1] = (toSet & 0x00FF);
	
	return;
}

void SetBufferFromUInt32(uint32_t toSet, uint8_t *buff, uint32_t index)
{
	buff[index] = (toSet & 0xFF000000) >> 24;
	buff[index + 1] = (toSet & 0x00FF0000) >> 16;
	buff[index + 2] = (toSet & 0x0000FF00) >> 8;
	buff[index + 3] = (toSet & 0x000000FF);
	
	return;
}



