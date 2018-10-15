#include "FunctionGetLocation.h"
#include "../GsmLibConstants.h"

// default constructor
FunctionGetLocation::FunctionGetLocation()
{
	functionTimeout = 20000;
	gpsLat[0] =0;
	gpsLong[0] = 0;
} //FunctionGetLocation

ParserState FunctionGetLocation::IncomingLine(FixedString150& line)
{
//AT+CIPGSMLOC=1,1 // triangulate
//+CIPGSMLOC: 0,19.667806,49.978185,2014/03/20,14:12:27
//
//OK
// +CIPGSMLOC: 601

		
	if(line == F("OK"))
	{
		if (parseOk)
			return ParserState::Success;
		else
			return ParserState::Error;
	}
	
	
	if(line.length() > 30)
	{
		
		char *p = (char*)line.c_str()+14;
				
		int n = 0;
		while(n < line.length() && *(p+n) != ',')	
			n++;
			
		strncpy(gpsLong, p, n);
		
		gpsLong[n] = 0;
		
			
		p+=n+1;
			
		n=0;
		while(n < line.length() && *(p+n) != ',')
			n++;
		gpsLat[n] = 0;
		
		strncpy(gpsLat, p, n);
			
		parseOk = true;
	}
	
	return ParserState::None;
}

const __FlashStringHelper* FunctionGetLocation::getCommand()
{
	return F("AT+CIPGSMLOC=1,1");
}

const __FlashStringHelper* FunctionGetLocation::GetInitSequence()
{
		return F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\0AT+SAPBR=3,1,\"APN\",\"internet\"\0AT+SAPBR=1,1\0AT+SAPBR=2,1\0\0");
}
