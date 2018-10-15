#ifndef __SIM900CONTEXT_H__
#define __SIM900CONTEXT_H__

#include "SimcomGsmTypes.h"

struct Sim900Context
{
	uint16_t signalStrength;
	uint16_t signalErrorRate;
	FixedString20* _ipAddress;
	FixedStringBase* _operatorName;
	bool _isOperatorNameReturnedInImsiFormat;
	FixedString150*_ussdResponse;
	FixedString20* _imei;
	BatteryStatus* _batteryStatus;
	IncomingCallInfo *_callInfo;
	SimcomIpState _ipState;
}; //Sim900Context

#endif //__SIM900CONTEXT_H__
