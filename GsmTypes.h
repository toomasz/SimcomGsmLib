#ifndef _GSM_TYPES_H
#define _GSM_TYPES_H

#include <FixedString.h>

class IncomingCallInfo
{
public:
	IncomingCallInfo()
	{
		HasAtiveCall = false;
	}
	bool HasAtiveCall;
	FixedString<20> CallerNumber;

};


#endif