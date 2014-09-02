/* 
* FunctionGetImei.h
*
* Created: 2014-03-21 18:52:17
* Author: Tom
*/

#include "FunctionBase.h"
#ifndef __FUNCTIONGETIMEI_H__
#define __FUNCTIONGETIMEI_H__


class FunctionGetImei: public FunctionBase
{
public:
	char Imei[16];
	bool imeiOk;
	FunctionGetImei();
	int IncomingLine(unsigned char *line, int lineLength, uint8_t crc);
    const __FlashStringHelper* getCommand();

}; 

#endif //__FUNCTIONGETIMEI_H__
