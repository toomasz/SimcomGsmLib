#ifndef SIM900CONSTANTS_H_
#define SIM900CONSTANTS_H_

#include <inttypes.h>

#define AT_DEFAULT 0
#define AT_CIPSTATUS 1
#define AT_CSQ 2
#define AT_CIFSR 3
#define AT_CIPSTART 4
#define AT_SWITH_TO_COMMAND 5
#define AT_SWITCH_TO_DATA 6
#define AT_COPS 7
#define AT_CREG 8
#define AT_GSN 9
#define AT_CUSTOM_FUNCTION 10
#define AT_CIPSHUT 11
#define AT_CIPCLOSE 12
#define AT_CUSD 13
#define AT_CBC 14
#define AT_CLCC 15


#define CRC_OK 0xe5     //"OK" l:2
#define CRC_ERROR 0x34  //"ERROR" l:5
#define CRC_CSQ 0xc    //"+CSQ:" l:5

#define CRC_CONNECT_FAIL 0x54   //"CONNECT FAIL" l:12
#define CRC_CONNECT 0x1f        //"CONNECT" l:7
#define CRC_CME_ERROR 0x47     //"+CME ERROR:" l:11
#define CRC_PDP_DEACT 0x1b      //"+PDP: DEACT" l:11
#define CRC_CLOSED 0x42 //"CLOSED" l:6
#define CRC_CREG 0x7   //"+CREG:" l:6
#define CRC_COPS 0xa   //"+COPS:" l:6
#define CRC_NO_CARRIER 0x56     //"NO CARRIER" l:10
#define CRC_CBC 0x90 // "+CBC:" l:5


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

const int ResponseBufferSize = 160;

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
