/* 
* FunctionGetLocation.h
*
* Created: 2014-03-25 18:46:25
* Author: Tom
*/
#include "FunctionBase.h"

#ifndef __FUNCTIONGETLOCATION_H__
#define __FUNCTIONGETLOCATION_H__

// size in ram = 12+12+1
class FunctionGetLocation: public FunctionBase
{
	public:
	char gpsLong[12];
	char gpsLat[12];
	bool parseOk;
	
	FunctionGetLocation();
	int IncomingLine(unsigned char *line, int lineLength, uint8_t crc);
	const __FlashStringHelper* getCommand();
    virtual const __FlashStringHelper* GetInitSequence();

};
#endif //__FUNCTIONGETLOCATION_H__
