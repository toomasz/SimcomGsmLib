#pragma once

#include "SimcomAtCommands.h"
#include <FixedString.h>
#include <WString.h>
#include "Network/GsmAsyncSocket.h"
#include "Network/SocketManager.h"
#include "GsmLogger.h"
#include <vector>

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
	SocketManager _socketManager;
	GsmLogger& _logger;
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
		_socketManager.SetIsNetworkAvailable(_state == GsmState::ConnectedToGprs);
	}
	GsmState _state;
	bool GetVariablesFromModem();

public:
	char *ApnName;
	char* ApnUser;
	char* ApnPassword;
	GsmModule(SimcomAtCommands &gsm);

	GsmAsyncSocket* CreateSocket(uint8_t mux, ProtocolType protocolType)
	{
		return _socketManager.CreateSocket(mux, protocolType);
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
	void OnLog(GsmLogCallback onLog);

	void Loop();
	void Wait(uint64_t delayInMs)
	{
		_gsm.wait(delayInMs);
	}
};

