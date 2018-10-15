#ifndef SIM900CONSTANTS_H_
#define SIM900CONSTANTS_H_

#include <inttypes.h>

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
	Clcc
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


const int AT_DEFAULT_TIMEOUT = 1500;

const int _defaultBaudRates[] =
{
	460800,
	115200,
	19200,
	38400,
	230400,
	9600,	
	4800,			
	57600,
	2400,
	1200,	
	0
};

typedef void(*UpdateBaudRateCallback)(int baudRate);

#endif
