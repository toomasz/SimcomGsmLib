#include "GsmTcpip.h"
#include "OperatorNameHelper.h"


GsmTcpip::GsmTcpip(SimcomGsm &gsm): 
	_gsm(gsm), 
	_justConnectedToModem(false), 
	_state(GsmState::NoShield)
{
}

void GsmTcpip::Loop()
{	
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

	if (_gsm.GetRegistrationStatus(gsmRegStatus) == AtResultType::Timeout)
	{
		ChangeState(GsmState::NoShield);
		return;
	}

	if (_gsm.GetSignalQuality(signalQuality) == AtResultType::Timeout)
	{
		ChangeState(GsmState::NoShield);
		return;
	}
	if (_gsm.GetBatteryStatus(batteryInfo) == AtResultType::Timeout)
	{
		ChangeState(GsmState::NoShield);
		return;
	}
	if (OperatorNameHelper::GetRealOperatorName(_gsm, operatorName) == AtResultType::Timeout)
	{
		ChangeState(GsmState::NoShield);
		return;
	}
	if (_gsm.GetIncomingCall(callInfo) == AtResultType::Timeout)
	{
		ChangeState(GsmState::NoShield);
		return;
	}
	if (_gsm.GetIpState(ipStatus) == AtResultType::Timeout)
	{
		ChangeState(GsmState::NoShield);
		return;
	}



	if (gsmRegStatus == GsmRegistrationState::HomeNetwork || gsmRegStatus == GsmRegistrationState::Roaming)
	{
		if (ipStatus == SimcomIpState::IpProcessing)
		{
			ChangeState(GsmState::ConnectedToGprs);
		}
	}
	else if (gsmRegStatus == GsmRegistrationState::SearchingForNetwork)
	{
		ChangeState(GsmState::SearchingForNetwork);
	}
	else if (gsmRegStatus == GsmRegistrationState::RegistrationDenied)
	{
		ChangeState(GsmState::RegistrationDenied);
	}
	else if (gsmRegStatus == GsmRegistrationState::RegistrationUnknown)
	{
		ChangeState(GsmState::RegistrationUnknown);
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
			//gui.Clear();
			//display.setFont(ArialMT_Plain_10);

			_gsm.SetCipmux(true);
			_gsm.SetRxMode(true);
			_gsm.SetApn("virgin-internet", "", "");
			//display.drawString(0, 0, "Connecting to gprs..");
			//display.display();
			//delay(400);
			auto attachResult = _gsm.AttachGprs();
			if (attachResult == AtResultType::Timeout)
			{
				return;
			}
			if (attachResult == AtResultType::Success)
			{
				ChangeState(GsmState::ConnectedToGprs);
			}
		}
	}	
}


GsmTcpip::~GsmTcpip()
{
}
