#ifndef _PARSER_CONTEXT_H
#define _PARSER_CONTEXT_H

#include "SimcomGsmTypes.h"
#include "SequenceDetector.h"

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
	FixedString128* UssdResponse;
	FixedString32* Imei;
	BatteryStatus* BatteryInfo;
	IncomingCallInfo* CallInfo;
	SimcomIpState* IpState;	
	bool IsOperatorNameReturnedInImsiFormat;
	bool Cipmux;
	ConnectionInfo* CurrentConnectionInfo;
	GsmRegistrationState RegistrationStatus;
	uint16_t CregLac;
	uint16_t CregCellId;
	SimState SimStatus;
	bool IsRxManual;
	FixedStringBase* CipRxGetBuffer;
	uint16_t CiprxGetLeftBytesToRead;
	// number of bytes that are left to be read from connection
	uint16_t* CiprxGetAvailableBytes;

	bool CipQSend;

	CipsendStateType CipsendState;
	FixedStringBase* CipsendBuffer;
	uint16_t CipsendDataIndex;
	uint16_t CipsendDataLength;
	uint16_t *CipsendSentBytes;
	SequenceDetector CipsendDataEchoDetector;
	float *Temperature;
    uint16_t* lastSmsMessageIndex;
    FixedString512* smsMessage;
};

#endif
