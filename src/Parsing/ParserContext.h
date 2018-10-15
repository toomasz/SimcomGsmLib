#ifndef _PARSER_CONTEXT_H
#define _PARSER_CONTEXT_H

#include "SimcomGsmTypes.h"

struct ParserContext
{
	int16_t* _signalQuality;
	FixedString20* _ipAddress;
	FixedStringBase* _operatorName;
	bool _isOperatorNameReturnedInImsiFormat;
	FixedString150*_ussdResponse;
	FixedString20* _imei;
	BatteryStatus* _batteryStatus;
	IncomingCallInfo *_callInfo;
	SimcomIpState* _ipState;
};

#endif
