#include "GsmTcpip.h"
#include "OperatorNameHelper.h"


GsmTcpip::GsmTcpip(SimcomGsm &gsm): 
	_gsm(gsm), 
	_justConnectedToModem(false), 
	_state(GsmState::Initial)
{
}

bool GsmTcpip::GetVariablesFromModem()
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
	if (OperatorNameHelper::GetRealOperatorName(_gsm, operatorName) == AtResultType::Timeout)
	{
		return false;
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

void GsmTcpip::Loop()
{
	if (_state == GsmState::Initial)
	{
		ChangeState(GsmState::NoShield);
		return;
	}
	if (_state == GsmState::NoShield)
	{
		if (!_gsm.EnsureModemConnected(460800))
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
		_gsm.GetCipmux(cipmux);
		_gsm.Cipshut();
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
			Serial.printf("CIPQSEND = %d\n", cipQsend);
		}

		_gsm.SetCipmux(true);
		_gsm.SetRxMode(true);
		_gsm.SetApn("virgin-internet", "", "");
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
	if (_state == GsmState::ConnectingToGprs)
	{
		if (ipStatus == SimcomIpState::PdpDeact)
		{
			ChangeState(GsmState::ConnectingToGprs);
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


const __FlashStringHelper* GsmTcpip::StateToStr(GsmState state)
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