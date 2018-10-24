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

class GsmIp
{
public:
	GsmIp()
	{
		memset(_octets, 0, 4);		
	}
	uint8_t _octets[4];
	FixedString20 ToString()
	{
		FixedString20 str;
		str.appendFormat("%d.%d.%d.%d", _octets[0], _octets[1], _octets[2], _octets[3]);
		return str;
	}
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
	ConnectionInfo()
	{
		Mux = 0;
		Bearer = 0;
		Port = 0;
	}
	uint8_t Mux;
	uint8_t Bearer;
	ProtocolType Protocol;
	GsmIp RemoteAddress;	 
	uint16_t Port;
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
	None,
	Timeout,
	Success,
	Error,
	PartialSuccess,
	PartialError
};

enum class AtResultType : uint8_t
{
	Success,
	Error,
	Timeout
};

enum class GsmRegistrationState : uint8_t
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