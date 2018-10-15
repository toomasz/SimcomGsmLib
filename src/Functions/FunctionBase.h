/* 
* FunctionBase.h
*
* Created: 2014-03-21 18:42:22
* Author: Tom
*/

#include <inttypes.h>
#include <FixedString.h>

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
	
    virtual ParserState IncomingLine(FixedString150 &line)=0;
	virtual const __FlashStringHelper* getCommand() = 0;
	virtual const __FlashStringHelper* GetInitSequence()
	{
		return 0;
	}
	int functionTimeout;
};

#endif //__FUNCTIONBASE_H__
