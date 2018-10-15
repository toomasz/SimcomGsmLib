#ifndef __FUNCTIONBASE_H__
#define __FUNCTIONBASE_H__

#include <inttypes.h>
#include <FixedString.h>

#include "../SimcomGsmTypes.h"
#include <Arduino.h>

class FunctionBase
{
	public:
	bool IsReady; 
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
