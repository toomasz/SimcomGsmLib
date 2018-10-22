#ifndef _SIMCOM_GSM_TYPES_H
#define _SIMCOM_GSM_TYPES_H

#include <FixedString.h>
#include <stdint.h>

typedef void(*UpdateBaudRateCallback)(int baudRate);

struct BatteryStatus
{
	double Voltage;
	uint8_t Percent;
};

enum class ConnectionState : uint8_t
{
	Initial,
	Connecting,
	Connected,
	RemoteClosing,
	Closing,
	Closed
};

enum class ProtocolType : uint8_t
{
	Tcp,
	Udp
};

class ConnectionInfo
{
public:
	uint8_t Mux;
	uint8_t Bearer;
	ProtocolType Protocol;
	IPAddress RemoteAddress;
	int Port;
	ConnectionState State;
};

enum class RegistrationMode: uint8_t
{
	Automatic = 0,
	Manual = 1,
	ManualWithFallback = 4
};



enum class AtCommand : uint8_t
{
	Generic,
	Cipstatus,
	CipstatusSingleConnection,
	Csq,
	Cifsr,
	Cipstart,
	SwitchToCommand,
	SwitchToData,
	Cops,
	Creg,
	Gsn,
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
		HasIncomingCall = false;
	}
	bool HasIncomingCall;
	FixedString<20> CallerNumber;
};


#endif