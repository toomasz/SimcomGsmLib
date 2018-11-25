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

const __FlashStringHelper* ProtocolToStr(ProtocolType protocol)
{
	switch (protocol)
	{
	case ProtocolType::Tcp:
		return F("TCP");
	case ProtocolType::Udp:
		return F("UDP");
	default:
		return F("");
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