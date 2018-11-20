#include "GsmTcpip.h"
#include "OperatorNameHelper.h"


GsmTcpip::GsmTcpip(SimcomGsm &gsm): 
	_gsm(gsm), 
	_justConnectedToModem(false), 
	_state(GsmState::Initializing)
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
	if (_state == GsmState::Initializing)
	{
		ChangeState(GsmState::NoShield);
		return;
	}
	if (_state == GsmState::NoShield)
	{
		if (!_gsm.EnsureModemConnected(115200))
		{
			delay(1000);
			return;
		}
		Serial.println("Modem found");
		_justConnectedToModem = true;
		ChangeState(GsmState::SearchingForNetwork);
		return;
	}
	if (_state == GsmState::ConnectingToGprs)
	{
		_gsm.SetCipmux(true);
		_gsm.SetRxMode(true);
		_gsm.SetApn("virgin-internet", "", "");
		auto attachResult = _gsm.AttachGprs();
		if (attachResult == AtResultType::Timeout)
		{
			return;
		}
		if (attachResult == AtResultType::Success)
		{
			ChangeState(GsmState::ConnectedToGprs);
		}
		return;
	}

	if (_justConnectedToModem)
	{
		_justConnectedToModem = false;
		_gsm.Cipshut();
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

	if (!GetVariablesFromModem())
	{
		ChangeState(GsmState::NoShield);
		return;
	}

	switch (gsmRegStatus)
	{
		case GsmRegistrationState::HomeNetwork:
		case GsmRegistrationState::Roaming:
		{
			if (ipStatus == SimcomIpState::IpProcessing || ipStatus == SimcomIpState::IpStatus)
			{
				ChangeState(GsmState::ConnectedToGprs);
			}
			break;
		}
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

	auto getIpResult = _gsm.GetIpAddress(ipAddress);
	if (getIpResult == AtResultType::Timeout)
	{
		ChangeState(GsmState::NoShield);
		return;
	}
	bool hasIpAddress = getIpResult == AtResultType::Success;

	if (!hasIpAddress)
	{
		_gsm.Cipshut();
	}

	if (gsmRegStatus == GsmRegistrationState::Roaming || 
		gsmRegStatus == GsmRegistrationState::HomeNetwork)
	{
		if (!hasIpAddress)
		{
			ChangeState(GsmState::ConnectingToGprs);
			return;
		}
	}

}