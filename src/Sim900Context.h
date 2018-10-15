#ifndef __SIM900CONTEXT_H__
#define __SIM900CONTEXT_H__

#include "SimcomGsmTypes.h"

struct Sim900Context
{
	uint16_t signalStrength;
	uint16_t signalErrorRate;
	uint16_t batteryPercent;
	uint16_t batteryVoltage;
	int registrationStatus;
	char ipAddress[16];
	FixedStringBase* _operatorName;
	bool _isOperatorNameReturnedInImsiFormat;
	char imei[16];
	uint16_t cellId;
	uint16_t lac;
	char *buffer_ptr;
	int buffer_size;
	IncomingCallInfo _callInfo;
	SimcomIpState _ipState;
}; //Sim900Context

#endif //__SIM900CONTEXT_H__
