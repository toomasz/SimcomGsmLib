/*
 * Sim900Constants.h
 *
 * Created: 2014-02-20 20:33:24
 *  Author: Tomasz Œcis³owicz
 */ 


#ifndef SIM900CONSTANTS_H_
#define SIM900CONSTANTS_H_

const int S900_NONE = -100;
const int  S900_TIMEOUT = -2;
const int  S900_ERR = -1;
const int  S900_OK = 0;

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

//GetRegistrationStatus result codes
#define SEARCHING_FOR_NETWORK0 0
#define HOME_NETWORK 1
#define SEARCHING_FOR_NETWORK 2
#define REGISTRATION_DENIED 3
#define REGISTRATION_UNKNOWN 4
#define ROAMING 5


const int AT_DEFAULT_TIMEOUT = 3000;

const int DATA_BUFFER_SIZE = 40;
const int ResponseBufferSize = 110;

#endif /* SIM900CONSTANTS_H_ */