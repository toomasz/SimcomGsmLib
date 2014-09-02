/* 
* FunctionGetImei.cpp
*
* Created: 2014-03-21 18:52:17
* Author: Tom
*/


#include "FunctionGetImei.h"

// default constructor
FunctionGetImei::FunctionGetImei()
{
	for(int i=0; i < 16; i++)
		this->Imei[i] = '0';
	Imei[15]=0;
	imeiOk = false;
} //FunctionGetImei

int FunctionGetImei::IncomingLine(unsigned char *line, int lineLength, uint8_t crc)
{
	
	if(imeiOk == false)
	{
		imeiOk = true;
		for(int i=0; i < lineLength; i++)
		if(line[i] < '0' || line[i] > '9')
			imeiOk = false;
	}
	
	if(imeiOk && crc == CRC_OK)
		return S900_OK;
	
	if(imeiOk)	
		strncpy(Imei, (char*)line, lineLength);
	

	return S900_NONE;
}

const __FlashStringHelper* FunctionGetImei::getCommand()
{
	return F("AT+GSN");
}
