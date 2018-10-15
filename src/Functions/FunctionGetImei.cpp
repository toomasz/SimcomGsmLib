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

ParserState FunctionGetImei::IncomingLine(FixedString150 &line)
{
	if(imeiOk == false)
	{
		imeiOk = true;
		for(int i=0; i < line.length(); i++)
		if(line.c_str()[i] < '0' || line.c_str()[i] > '9')
			imeiOk = false;
	}
	
	if (imeiOk && line == F("OK"))
	{
		return ParserState::Success;
	}
	
	if (imeiOk)
	{
		strncpy(Imei, (char*)line.c_str(), line.length());
	}

	return ParserState::None;
}

const __FlashStringHelper* FunctionGetImei::getCommand()
{
	return F("AT+GSN");
}
