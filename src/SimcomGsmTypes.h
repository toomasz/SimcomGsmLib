#ifndef _SIMCOM_GSM_TYPES_H
#define _SIMCOM_GSM_TYPES_H

#include <FixedString.h>
#include <inttypes.h>

typedef void(*UpdateBaudRateCallback)(int baudRate);

struct BatteryStatus
{
	double Voltage;
	uint8_t Percent;
};

enum class AtCommand : uint8_t
{
	Generic,
	Cipstatus,
	Csq,
	Cifsr,
	Cipstart,
	SwitchToCommand,
	SwitchToData,
	Cops,
	Creg,
	Gsn,
	CustomFunction,
	Cipshut,
	Cipclose,
	Cusd,
	Cbc,
	Clcc,
	Cipmux
};

enum class SimcomIpState : uint8_t
{
	IpInitial,
	IpStart,
	IpConfig,
	IpGprsact,
	IpStatus,
	TcpConnecting,
	TcpClosed,
	PdpDeact,
	ConnectOk,
	IpProcessing,
	Unknown
};

enum class ParserState : uint8_t
{
	Success,
	Error,
	Timeout,
	None
};

enum class AtResultType : uint8_t
{
	Success,
	Error,
	Timeout
};

enum class GsmNetworkStatus : uint8_t
{
	SearchingForNetwork,
	HomeNetwork,
	RegistrationDenied,
	RegistrationUnknown,
	Roaming
};

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