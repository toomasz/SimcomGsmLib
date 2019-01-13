#ifndef _GSMLIBHELPERS_H
#define _GSMLIBHELPERS_H

#include <WString.h>
#include "SimcomGsmTypes.h"
#include "Network\GsmAsyncSocket.h"
#include <FixedString.h>

const __FlashStringHelper* SocketEventTypeToStr(SocketEventType socketEvent);
const __FlashStringHelper* SocketStateToStr(SocketStateType state);
const __FlashStringHelper* IpStatusToStr(SimcomIpState ipStatus);
const __FlashStringHelper* RegStatusToStr(GsmRegistrationState state);
const char* ProtocolToStr(ProtocolType protocol);
const __FlashStringHelper* ConnectionStateToStr(ConnectionState state);
void BinaryToString(FixedStringBase&source, FixedStringBase& target);

class IntervalTimer
{
	uint64_t _ticks;
	int _delay;
	bool _isElapsed;
	void Tick();
	void SetElapsed();	
public:
	void SetDelay(int delay);
	IntervalTimer(int delay);
	bool IsElapsed();
};

#endif