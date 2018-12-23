#include "OperatorNameHelper.h"
#include "GsmModule.h"


GsmModule::GsmModule(SimcomAtCommands &gsm):
	_gsm(gsm), 
	_socketManager(gsm, gsm.Logger()),
	_state(GsmState::Initial),
	BaudRate(115200)	
{
}

bool GsmModule::GetVariablesFromModem()
{
	if (_gsm.GetRegistrationStatus(gsmRegStatus) == AtResultType::Timeout)
	{
		return false;
	}

	if (_gsm.GetSignalQuality(signalQuality) == AtResultType::Timeout)
	{
		return false;
	}
	if (_gsm.GetBatteryStatus(batteryInfo) == AtResultType::Timeout)
	{
		return false;
	}
	if (gsmRegStatus == GsmRegistrationState::HomeNetwork || gsmRegStatus == GsmRegistrationState::Roaming)
	{
		if (OperatorNameHelper::GetRealOperatorName(_gsm, operatorName) == AtResultType::Timeout)
		{
			return false;
		}
	}
	if (_gsm.GetIncomingCall(callInfo) == AtResultType::Timeout)
	{
		return false;
	}
	if (_gsm.GetIpState(ipStatus) == AtResultType::Timeout)
	{
		return false;
	}
	return true;
}

void GsmModule::Loop()
{
	if (_state == GsmState::Initial)
	{
		ChangeState(GsmState::NoShield);
		return;
	}
	if (_state == GsmState::NoShield)
	{
		if (!_gsm.EnsureModemConnected(BaudRate))
		{
			delay(500);
			return;
		}
		ChangeState(GsmState::Initializing);
		return;
	}

	if (_state == GsmState::Initializing)
	{
		bool cipmux;
		_gsm.FlightModeOn();
		_gsm.FlightModeOff();
		_gsm.wait(5000);
		_gsm.GetCipmux(cipmux);
		//_gsm.Cipshut();
		ChangeState(GsmState::SearchingForNetwork);
		return;
	}

	if (_state == GsmState::SimError)
	{
		auto simResult = _gsm.GetSimStatus(simStatus);
		if (simResult == AtResultType::Timeout)
		{
			ChangeState(GsmState::NoShield);
			return;
		}
		if (simStatus == SimState::Ok)
		{
			ChangeState(GsmState::Initializing);
			return;
		}
		_gsm.FlightModeOn();
		_gsm.FlightModeOff();
		return;
	}

	if (!GetVariablesFromModem())
	{
		ChangeState(GsmState::NoShield);
		return;
	}
	if (_state == GsmState::SearchingForNetwork)
	{
		auto regStatusResult = _gsm.GetRegistrationStatus(gsmRegStatus);
		if (regStatusResult == AtResultType::Timeout)
		{
			ChangeState(GsmState::NoShield);
			return;
		}
		if (regStatusResult == AtResultType::Success)
		{
			if (gsmRegStatus == GsmRegistrationState::HomeNetwork || gsmRegStatus == GsmRegistrationState::Roaming)
			{
				ChangeState(GsmState::ConnectingToGprs);
				return;
			}
		}
	}

	if (_state == GsmState::ConnectingToGprs)
	{
		_gsm.Cipshut();

		if (_gsm.SetSipQuickSend(true) == AtResultType::Success)
		{
			Serial.println("Successfully set CIPQSEND to 1\n");
		}

		bool cipQsend;
		if (_gsm.GetCipQuickSend(cipQsend) == AtResultType::Success)
		{
		}

		_gsm.SetCipmux(true);
		_gsm.SetRxMode(true);
		auto apnResult = _gsm.SetApn("virgin-internet", "", "");
		if (apnResult == AtResultType::Timeout)
		{
			ChangeState(GsmState::NoShield);
			return;
		}
		auto attachResult = _gsm.AttachGprs();
		if (attachResult == AtResultType::Timeout)
		{
			ChangeState(GsmState::NoShield);
			return;
		}
		if (attachResult != AtResultType::Success)
		{
			return;
		}
		auto ipAddressResult = _gsm.GetIpAddress(ipAddress);
		if (ipAddressResult == AtResultType::Timeout)
		{
			ChangeState(GsmState::NoShield);
			return;
		}
		if (ipAddressResult != AtResultType::Success)
		{
			return;
		}
		ChangeState(GsmState::ConnectedToGprs);
		return;
	}
	if (_state == GsmState::ConnectedToGprs)
	{
		if (ipStatus == SimcomIpState::PdpDeact)
		{
			ChangeState(GsmState::ConnectingToGprs);
			return;
		}
		if (!_socketManager.ReadDataFromSockets())
		{
			_gsm.Logger().Log(F("Timeout while trying to read from on of sockets"));
			ChangeState(GsmState::NoShield);
			return;
		}
	}

	if (_gsm.GetSimStatus(simStatus) == AtResultType::Success)
	{
		if (simStatus != SimState::Ok)
		{
			ChangeState(GsmState::SimError);
			delay(500);
			return;
		}
	}

	if (_state == GsmState::RegistrationDenied ||
		_state == GsmState::RegistrationUnknown)
	{
		switch (gsmRegStatus)
		{
			case GsmRegistrationState::SearchingForNetwork:
			{
				ChangeState(GsmState::SearchingForNetwork);
				break;
			}
			case GsmRegistrationState::HomeNetwork:
			case GsmRegistrationState::Roaming:
			{
				ChangeState(GsmState::ConnectingToGprs);
				break;
			}

			default:
				break;
		}
	}

	switch (gsmRegStatus)
	{		
		case GsmRegistrationState::SearchingForNetwork:
		{
			ChangeState(GsmState::SearchingForNetwork);
			break;
		}
		case GsmRegistrationState::RegistrationDenied:
		{
			ChangeState(GsmState::RegistrationDenied);
			break;
		}
		case GsmRegistrationState::RegistrationUnknown:
		{
			ChangeState(GsmState::RegistrationUnknown);
			break;
		}
	default:
		break;
	}
}


const __FlashStringHelper* GsmModule::StateToStr(GsmState state)
{
	switch (state)
	{
	case GsmState::Initial: return F("Initial");
	case GsmState::NoShield: return F("NoShield");
	case GsmState::Initializing: return F("Initializing");
	case GsmState::SimError: return F("SimError");
	case GsmState::SearchingForNetwork: return F("SearchingForNetwork");
	case GsmState::RegistrationDenied: return F("RegistrationDenied");
	case GsmState::RegistrationUnknown: return F("RegistrationUnknown");
	case GsmState::ConnectingToGprs: return F("ConnectingToGprs");
	case GsmState::ConnectedToGprs: return F("ConnectedToGprs");
	default: return F("Unknown");
	}
}

int GsmModule::GarbageOnSerialDetected()
{
	return _gsm.GarbageOnSerialDetected();
}
