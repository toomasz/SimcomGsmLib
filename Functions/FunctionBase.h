/* 
* FunctionBase.h
*
* Created: 2014-03-21 18:42:22
* Author: Tom
*/

#include <inttypes.h>

#include "../Sim900Constants.h"
#include <Arduino.h>

#ifndef __FUNCTIONBASE_H__
#define __FUNCTIONBASE_H__



class FunctionBase
{
	public:
	bool IsReady; 
    //508 551 395
	FunctionBase();
	
    virtual int IncomingLine(unsigned char *line, int lineLength, uint8_t crc)=0;
	virtual const __FlashStringHelper* getCommand() = 0;
	virtual const __FlashStringHelper* GetInitSequence()
	{
		return 0;
	}
	int functionTimeout;
};

#endif //__FUNCTIONBASE_H__
