/* 
* S900Socket.cpp
*
* Created: 2014-10-18 16:21:55
* Author: Tom
*/


#include "S900Socket.h"


uint8_t S900Socket::write(char *data, int length)
{
	
}
/* read count bytes from socket
   returns:
   number of read bytes on success
   -1 when \r\nCLOSED\r\n is detected
   -2 when \r\n+PDP: DEACT\r\n is detected
*/
uint16_t S900Socket::read_blocking(char *data, uint16_t count, unsigned long timeout)
{
	unsigned long startTime = millis();
	uint16_t dataReceived = 0;
	
	
	return dataReceived;
}

