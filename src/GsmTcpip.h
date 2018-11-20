#pragma once

#include "SimcomGsmLib.h"
#include <FixedString.h>


enum class GsmState
{
	Initializing,
	NoShield,
	SimError,
	SearchingForNetwork,
	RegistrationDenied,
	RegistrationUnknown,
	ConnectingToGprs,
	ConnectedToGprs,
};

class GsmTcpip
{
	SimcomGsm& _gsm;
	bool _justConnectedToModem;
	void ChangeState(GsmState newState)
	{
		_state = newState;
	}
	GsmState _state;

public:
	GsmState GetState()
	{
		return _state;
	}
	int16_t signalQuality;
	BatteryStatus batteryInfo;
	FixedString20 operatorName;
	IncomingCallInfo callInfo;
	GsmRegistrationState gsmRegStatus;
	GsmIp ipAddress;
	SimcomIpState ipStatus;
	SimState simStatus;

	GsmTcpip(SimcomGsm &gsm);
	void Loop();
};

