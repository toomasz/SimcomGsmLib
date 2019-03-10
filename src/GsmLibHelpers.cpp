#include "GsmLibHelpers.h"
#include <pgmspace.h>
#include <stdio.h>

const __FlashStringHelper* IpStatusToStr(SimcomIpState ipStatus)
{
	switch (ipStatus)
	{
	case SimcomIpState::IpInitial:	return F("IP_INITIAL");
	case SimcomIpState::IpStart:	return F("IP_START");
	case SimcomIpState::IpConfig:	return F("IP_CONFIG");
	case SimcomIpState::IpGprsact:	return F("IP_GPRSACT");
	case SimcomIpState::IpStatus:	return F("IP_STATUS");
	case SimcomIpState::TcpConnecting:return F("TCP_CONNECTING");
	case SimcomIpState::TcpClosed:	return F("TCP_CLOSED");
	case SimcomIpState::PdpDeact:	return F("PDP_DEACT");
	case SimcomIpState::ConnectOk:	return F("CONNECT_OK");
	case SimcomIpState::IpProcessing: return F("IP_PROCESSING");
	default: return F("Unknown");
	}
}

const __FlashStringHelper * RegStatusToStr(GsmRegistrationState state)
{
	switch (state)
	{
	case GsmRegistrationState::HomeNetwork: return F("HomeNetwork");
	case GsmRegistrationState::RegistrationDenied: return F("RegistrationDenied");
	case GsmRegistrationState::RegistrationUnknown: return F("RegistrationUnknown");
	case GsmRegistrationState::Roaming: return F("Roaming");
	case GsmRegistrationState::SearchingForNetwork: return F("SearchingForNetwork");
	default: return F("Unknown");
	}
}

const char* ProtocolToStr(ProtocolType protocol)
{
	switch (protocol)
	{
	case ProtocolType::Tcp:
		return "TCP";
	case ProtocolType::Udp:
		return "UDP";
	default:
		return "";
	}
}

const __FlashStringHelper * SocketEventTypeToStr(SocketEventType socketEvent)
{
	switch (socketEvent)
	{
	case SocketEventType::ConnectBegin: return F("ConnectBegin");
	case SocketEventType::ConnectFailed: return F("ConnectFailed");
	case SocketEventType::ConnectSuccess: return F("ConnectSuccess");
	case SocketEventType::Disconnecting: return F("Disconnecting");
	case SocketEventType::Disconnected: return F("Disconnected");
	default: return F("Unknown");
	}
}

const __FlashStringHelper* SocketStateToStr(SocketStateType state)
{
	switch (state)
	{
	case SocketStateType::Closed: return F("Closed");
	case SocketStateType::Connecting: return F("Connecting");
	case SocketStateType::Connected: return F("Connected");
	case SocketStateType::Closing: return F("Closing");
	default: return F("Unknown");
	}
}

const __FlashStringHelper* ConnectionStateToStr(ConnectionState state)
{
	switch (state)
	{
	case ConnectionState::Initial: return F("Initial");
	case ConnectionState::Connected: return F("Connected");
	case ConnectionState::Connecting: return F("Connecting");
	case ConnectionState::RemoteClosing: return F("RemoteClosing");
	case ConnectionState::Closing: return F("Closing");
	case ConnectionState::Closed: return F("Closed");
	default: return F("Unknown");
	}
}

void BinaryToString(FixedStringBase&source, FixedStringBase& target)
{
	for (int i = 0; i < source.length(); i++)
	{
		const char c = source[i];
		if (target.freeBytes() < 3)
		{
			return;
		}
		if(isprint(c))
		{
			target.append(c);
		}
		else if(c == '\r')
		{
			target.append("\\r");
		}
		else if (c == '\n')
		{
			target.append("\\n");
		}
		else
		{
			target.appendFormat("\\%x", c);
		}
	}
}


IntervalTimer::IntervalTimer(int delay)
{
	_delay = delay;
	_ticks = 0;
	_isElapsed = false;
}

void IntervalTimer::SetDelay(int delay)
{
	_delay = delay;
}

void IntervalTimer::Tick()
{
	auto ms = millis();
	if (ms - _ticks < _delay)
	{
		return;
	}
	_isElapsed = true;
	_ticks = ms;

}

void IntervalTimer::SetElapsed()
{
	_isElapsed = true;
}

bool IntervalTimer::IsElapsed()
{
	Tick();
	auto isElapsed = _isElapsed;
	_isElapsed = false;
	return isElapsed;
}
