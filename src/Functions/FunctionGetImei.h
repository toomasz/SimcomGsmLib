#ifndef __FUNCTIONGETIMEI_H__
#define __FUNCTIONGETIMEI_H__

#include <FixedString.h>
#include "FunctionBase.h"

class FunctionGetImei: public FunctionBase
{
public:
	char Imei[16];
	bool imeiOk;
	FunctionGetImei();
	ParserState IncomingLine(FixedString150 &line);
    const __FlashStringHelper* getCommand();

}; 

#endif //__FUNCTIONGETIMEI_H__
