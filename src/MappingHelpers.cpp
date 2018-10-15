#include "MappingHelpers.h"

#include <pgmspace.h>
#include <stdio.h>
#include <WString.h>

struct IpStatusEntry
{
	const __FlashStringHelper *str;
	SimcomIpState status;
};
struct IpStatusEntry IpStatusMap[] =
{
	F("STATE: IP INITIAL"), SimcomIpState::IpInitial,
	F("STATE: IP START"), SimcomIpState::IpStart,
	F("STATE: IP CONFIG"), SimcomIpState::IpConfig,
	F("STATE: IP GPRSACT"), SimcomIpState::IpGprsact,
	F("STATE: IP STATUS"), SimcomIpState::IpStatus,
	F("STATE: TCP CONNECTING"), SimcomIpState::TcpConnecting,
	F("STATE: TCP CLOSED"), SimcomIpState::TcpClosed,
	F("STATE: PDP DEACT"), SimcomIpState::PdpDeact,
	F("STATE: CONNECT OK"), SimcomIpState::ConnectOk,
	F("STATE: IP PROCESSING"), SimcomIpState::IpProcessing,
	0,SimcomIpState::Unknown
};

GsmNetworkStatus CregToNetworkStatus(uint16_t status)
{
	switch (status)
	{
	case 0:
	case 2:
		return GsmNetworkStatus::SearchingForNetwork;
	case 1:
		return GsmNetworkStatus::HomeNetwork;
	case 3:
		return GsmNetworkStatus::RegistrationDenied;
	case 4:
		return GsmNetworkStatus::RegistrationUnknown;
	case 5:
		return GsmNetworkStatus::Roaming;
	default:
		return GsmNetworkStatus::RegistrationUnknown;

	}
}

bool ParseIpStatus(const char *str, SimcomIpState &status)
{
	int i = 0;

	auto key = IpStatusMap[i].str;
	while (key)
	{
		if (strcmp_P(str, (PGM_P)key) == 0)
		{
			status = IpStatusMap[i].status;
			return true;
		}
		key = IpStatusMap[++i].str;
	}
	return false;
}


