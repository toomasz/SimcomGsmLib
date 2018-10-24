#include "MappingHelpers.h"

#include <pgmspace.h>
#include <stdio.h>
#include <WString.h>

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

