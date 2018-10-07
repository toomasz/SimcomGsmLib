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


#define CRC_OK 0xe5     //"OK" l:2
#define CRC_ERROR 0x34  //"ERROR" l:5
#define CRC_CSQ 0xc    //"+CSQ:" l:5
#define CRC_STATE_IP_INITIAL 0x25       //"STATE: IP INITIAL" l:17
#define CRC_STATE_IP_START 0x34 //"STATE: IP START" l:15
#define CRC_STATE_IP_CONFIG 0x66        //"STATE: IP CONFIG" l:16
#define CRC_STATE_IP_GPRSACT 0x73       //"STATE: IP GPRSACT" l:17
#define CRC_STATE_IP_STATUS 0xf3        //"STATE: IP STATUS" l:16
#define CRC_STATE_TCP_CONNECTING 0xd0   //"STATE: TCP CONNECTING" l:21
#define CRC_STATE_TCP_CLOSED 0x61       //"STATE: TCP CLOSED" l:17
#define CRC_STATE_PDP_DEACT 0x2b        //"STATE: PDP DEACT" l:16
#define CRC_STATE_CONNECT_OK 0xec       //"STATE: CONNECT OK" l:17
#define CRC_CONNECT_FAIL 0x54   //"CONNECT FAIL" l:12
#define CRC_CONNECT 0x1f        //"CONNECT" l:7
#define CRC_CME_ERROR 0x47     //"+CME ERROR:" l:11
#define CRC_PDP_DEACT 0x1b      //"+PDP: DEACT" l:11
#define CRC_CLOSED 0x42 //"CLOSED" l:6
#define CRC_CREG 0x7   //"+CREG:" l:6
#define CRC_COPS 0xa   //"+COPS:" l:6
#define CRC_NO_CARRIER 0x56     //"NO CARRIER" l:10
#define CRC_CBC 0x90 // "+CBC:" l:5

//GetIpStatus() AT+CIPSTATUS result codes
#define IP_INITIAL 0x25
#define IP_START 0x34
#define IP_CONFIG 0x66
#define IP_GPRSACT 0x73
#define IP_STATUS 0xf3
#define TCP_CONNECTING 0xd0
#define TCP_CLOSED 0x61
#define PDP_DEACT 0x2b
#define CONNECT_OK 0xec

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

const int DATA_BUFFER_SIZE = 40;
const int ResponseBufferSize = 160;

const int _defaultBaudRates[] =
{
	1200,
	2400,
	4800,
	9600,
	19200,
	38400,
	57600,
	115200,
	230400,
	460800,
	0
};

#endif /* SIM900CONSTANTS_H_ */
