/* 
* S900Socket.h
*
* Created: 2014-10-18 16:21:55
* Author: Tom
*/


#ifndef __S900SOCKET_H__
#define __S900SOCKET_H__

#include "Sim900.h"
#include <Arduino.h>


const char ClosedString[]  = "\r\nCLOSED\r\n";
const char PdpDeactString[]  = "\r\n+PDP: DEACT\r\n";

const uint8_t SOCKET_OPEN = 0;
const uint8_t SOCKET_CLOSED = 1;
const uint8_t SOCKET_PDP_DEACT = 2;

class S900Socket
{
//variables
	SequenceDetector detectorClosed;
	SequenceDetector detectorPdpDeact;
public:
	S900Socket():detectorClosed(ClosedString), detectorPdpDeact(PdpDeactString)
	{
		socket_state = SOCKET_OPEN;
	}
	Sim900 *s900;
	uint8_t socket_state;
	uint16_t read_blocking(char *data, uint16_t length, unsigned long  timeout);
	uint8_t write(char *data, int length);
private:
}; //S900Socket

#endif //__S900SOCKET_H__
