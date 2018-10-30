#ifndef _PARSER_CONTEXT_H
#define _PARSER_CONTEXT_H

#include "SimcomGsmTypes.h"

struct ParserContext
{
	ParserContext()
	{
		Cipmux = false;
		IsOperatorNameReturnedInImsiFormat = false;
		IsRxManual = false;
	}
	int16_t* CsqSignalQuality;
	GsmIp* IpAddress;
	FixedStringBase* OperatorName;
	uint16_t operatorSelectionMode;
	FixedString150* UssdResponse;
	FixedString20* Imei;
	BatteryStatus* BatteryInfo;
	IncomingCallInfo* CallInfo;
	SimcomIpState* IpState;

	bool IsOperatorNameReturnedInImsiFormat;
	bool Cipmux;
	ConnectionInfo* CurrentConnectionInfo;
	GsmRegistrationState RegistrationStatus;
	SimState SimStatus;
	bool IsRxManual;
	FixedStringBase* CipRxGetBuffer;
	uint16_t CiprxGetLeftBytesToRead;
};

#endif
