#ifndef __SIM900CONTEXT_H__
#define __SIM900CONTEXT_H__

#include "SimcomGsmTypes.h"

struct Sim900Context
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

#endif //__SIM900CONTEXT_H__
