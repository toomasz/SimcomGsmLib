/* 
* FunctionGetLocation.cpp
*
* Created: 2014-03-25 18:46:25
* Author: Tom
*/


#include "FunctionGetLocation.h"

// default constructor
FunctionGetLocation::FunctionGetLocation()
{
	functionTimeout = 20000;
	gpsLat[0] =0;
	gpsLong[0] = 0;
} //FunctionGetLocation

int FunctionGetLocation::IncomingLine(unsigned char *line, int lineLength, uint8_t crc)
{
//AT+CIPGSMLOC=1,1 // triangulate
//+CIPGSMLOC: 0,19.667806,49.978185,2014/03/20,14:12:27
//
//OK
// +CIPGSMLOC: 601

		
	if(crc == CRC_OK)
	{
		if(parseOk)
			return S900_OK;
		else
			return S900_ERR;
	}
	
	
	if(lineLength > 30)
	{
		
		char *p = (char*)line+14;
				
		int n = 0;
		while(n < lineLength && *(p+n) != ',')	
			n++;
			
		strncpy(gpsLong, p, n);
		
		gpsLong[n] = 0;
		
			
		p+=n+1;
			
		n=0;
		while(n < lineLength && *(p+n) != ',')
			n++;
		gpsLat[n] = 0;
		
		strncpy(gpsLat, p, n);
			
		parseOk = true;
	}
	
	return S900_NONE;
}

const __FlashStringHelper* FunctionGetLocation::getCommand()
{
	return F("AT+CIPGSMLOC=1,1");
}

const __FlashStringHelper* FunctionGetLocation::GetInitSequence()
{
		return F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\0AT+SAPBR=3,1,\"APN\",\"internet\"\0AT+SAPBR=1,1\0AT+SAPBR=2,1\0\0");
}
