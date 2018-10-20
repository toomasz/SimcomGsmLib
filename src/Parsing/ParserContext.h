#ifndef _PARSER_CONTEXT_H
#define _PARSER_CONTEXT_H

#include "SimcomGsmTypes.h"

struct ParserContext
{
	int16_t* CsqSignalQuality;
	FixedString20* IpAddress;
	FixedStringBase* OperatorName;
	int16_t operatorSelectionMode;
	FixedString150* UssdResponse;
	FixedString20* Imei;
	BatteryStatus* BatteryInfo;
	IncomingCallInfo *CallInfo;
	SimcomIpState* IpState;
	bool IsOperatorNameReturnedInImsiFormat;
	bool Cipmux;
};

#endif
