#include "OperatorNameHelper.h"
#include "GsmModule.h"
#include "GsmLibHelpers.h"


GsmModule::GsmModule(SimcomAtCommands &gsm):
	_gsm(gsm), 
	_socketManager(gsm, gsm.Logger()),
	_logger(gsm.Logger()),
	_state(GsmState::Initial),
	ApnName(""),
	ApnUser(""),
	ApnPassword(""),
	BaudRate(115200)	
{
	Serial.println("GsmModule::GsmModule");
	_gsm.OnGsmModuleEvent(this, [](void*ctx, GsmModuleEventType eventType) 
	{
		reinterpret_cast<GsmModule*>(ctx)->OnGsmModuleEvent(eventType);
	});
}
void GsmModule::OnGsmModuleEvent(GsmModuleEventType eventType)
{
	if (eventType == GsmModuleEventType::OverVoltagePowerDown)
	{
		_error = "Over voltage power down";
		ChangeState(GsmState::Error);
	}
	if (eventType == GsmModuleEventType::UnderVoltagePowerDown)
	{
		_error = "Under voltage power down";
		ChangeState(GsmState::Error);
	}
	if (eventType == GsmModuleEventType::OverVoltageWarning)
	{
		Serial.println("Voltage is too high!");
	}
}
bool GsmModule::ReadModemProperties()
{
	static IntervalTimer getVariablesTimer(GetPropertiesInterval);
	if (!getVariablesTimer.IsElapsed())
	{
		return true;
	}
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
	if (_state == GsmState::ConnectedToGprs)
	{
		if (_gsm.GetIpState(ipStatus) == AtResultType::Timeout)
		{
			return false;
		}
	}
	return true;
}

void GsmModule::Loop()
{
	static IntervalTimer loopIntervalTimer(TickInterval);
	if (!loopIntervalTimer.IsElapsed())
	{
		return;
	}

	if (_state == GsmState::Error)
	{
		return;
	}
	if (GarbageOnSerialDetected())
	{
		_error = "Serial garbage detected";
		ChangeState(GsmState::Error);
		return;
	}

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
		if (_gsm.GetCipmux(cipmux) == AtResultType::Timeout)
		{
			ChangeState(GsmState::NoShield);
			return;
		}
		if (cipmux)
		{
			if (_gsm.FlightModeOn() == AtResultType::Timeout)
			{
				ChangeState(GsmState::NoShield);
				return;
			}
			if (_gsm.FlightModeOff() == AtResultType::Timeout)
			{
				ChangeState(GsmState::NoShield);
				return;
			}
			_gsm.wait(5000);
		}
		
		//_gsm.Cipshut();
		ChangeState(GsmState::SearchingForNetwork);
		return;
	}

	if (_state == GsmState::SimError)
	{
		const auto simResult = _gsm.GetSimStatus(simStatus);
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

	if (!ReadModemProperties())
	{
		ChangeState(GsmState::NoShield);
		return;
	}
	if (_state == GsmState::SearchingForNetwork)
	{
		const auto regStatusResult = _gsm.GetRegistrationStatus(gsmRegStatus);
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
		_logger.Log(F("Connecting to GPRS"));
		_logger.Log(F("Executing CIPSHUT"));
		_gsm.Cipshut();

		_logger.Log(F("Executing CIPQSEND"));
		if (_gsm.SetSipQuickSend(true) == AtResultType::Timeout)
		{
			ChangeState(GsmState::NoShield);
			return;
		}

		bool cipQsend;
		if (_gsm.GetCipQuickSend(cipQsend) == AtResultType::Timeout)
		{
			ChangeState(GsmState::NoShield);
			return;
		}
		_logger.Log(F("Executing CIPMUX=1"));
		if (_gsm.SetCipmux(true) == AtResultType::Timeout)
		{
			ChangeState(GsmState::NoShield);
			return;
		}
		_logger.Log(F("Executing CIPRXGET=1"));
		if (_gsm.SetRxMode(true) == AtResultType::Timeout)
		{
			ChangeState(GsmState::NoShield);
			return;
		}
		_logger.Log(F("Executing CSTT"));
		auto apnResult = _gsm.SetApn(ApnName, ApnUser, ApnPassword);
		if (apnResult == AtResultType::Timeout)
		{
			ChangeState(GsmState::NoShield);
			return;
		}
		_logger.Log(F("Executing CIICR"));
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
		_logger.Log(F("Executing CIFSR"));
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
			_logger.Log(F("Timeout while trying to read data from socket"));
			ChangeState(GsmState::NoShield);
			return;
		}
	}
	static IntervalTimer simStatusTimer(SimStatusInterval);
	if (simStatusTimer.IsElapsed())
	{
		if (_gsm.GetSimStatus(simStatus) == AtResultType::Success)
		{
			if (simStatus != SimState::Ok)
			{
				ChangeState(GsmState::SimError);
				delay(500);
				return;
			}
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
void GsmModule::OnLog(GsmLogCallback onLog)
{
	_gsm.Logger().OnLog(onLog);
}

int GsmModule::GarbageOnSerialDetected()
{
	return _gsm.GarbageOnSerialDetected();
}
