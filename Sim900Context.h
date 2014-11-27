/* 
* Sim900Context.h
*
* Created: 2014-06-18 18:08:10
* Author: Tomasz Œcis³owicz
*/


#ifndef __SIM900CONTEXT_H__
#define __SIM900CONTEXT_H__

struct Sim900Context
{
	uint16_t signalStrength;
	uint16_t signalErrorRate;
	int registrationStatus;
	char ipAddress[16];
	char operatorName[20];
	char imei[16];
	uint16_t cellId;
	uint16_t lac;
	char *buffer_ptr;
	int buffer_size;
}; //Sim900Context

#endif //__SIM900CONTEXT_H__
