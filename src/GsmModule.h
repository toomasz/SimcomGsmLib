#pragma once

#include "SimcomAtCommands.h"
#include <FixedString.h>
#include <WString.h>
#include "GsmAsyncSocket.h"

enum class GsmState
{
	Initial,
	NoShield,
	Initializing,
	SimError,
	SearchingForNetwork,
	RegistrationDenied,
	RegistrationUnknown,
	ConnectingToGprs,
	ConnectedToGprs,
};



class GsmModule
{
	SimcomAtCommands& _gsm;
	bool _justConnectedToModem;
	
	void GetStateStringFromProg(char* stateStr, GsmState state)
	{
		strcpy_P(stateStr, (PGM_P)StateToStr(state));
	}

	void ChangeState(GsmState newState)
	{
		if (_state != newState)
		{
			char inState[20];
			char outState[20];
			GetStateStringFromProg(inState, _state);
			GetStateStringFromProg(outState, newState);
			_gsm.Logger().Log(F("State changed %s -> %s"), inState, outState);
		}
		_state = newState;
	}
	GsmState _state;
	bool GetVariablesFromModem();
public:
	GsmAsyncSocket* CreateSocket()
	{

	}
	uint64_t BaudRate;
	GsmState GetState()
	{
		return _state;
	}
	const __FlashStringHelper* StateToStr(GsmState state);
	int GarbageOnSerialDetected();
	int16_t signalQuality;
	BatteryStatus batteryInfo;
	FixedString20 operatorName;
	IncomingCallInfo callInfo;
	GsmRegistrationState gsmRegStatus;
	GsmIp ipAddress;
	SimcomIpState ipStatus;
	SimState simStatus;

	GsmModule(SimcomAtCommands &gsm);
	void Loop();
	void Wait(uint64_t delayInMs)
	{
		_gsm.wait(delayInMs);
	}
};

