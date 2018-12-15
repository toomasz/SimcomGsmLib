#include "ParsingHelpers.h"
#include "DelimParser.h"

bool ParsingHelpers::ParseRegistrationStatus(uint16_t status, GsmRegistrationState& state)
{
	switch (status)
	{
	case 0:
	case 2:
		state = GsmRegistrationState::SearchingForNetwork;
		return true;
	case 1:
		state = GsmRegistrationState::HomeNetwork;
		return true;
	case 3:
		state = GsmRegistrationState::RegistrationDenied;
		return true;
	case 4:
		state = GsmRegistrationState::RegistrationUnknown;
		return true;
	case 5:
		state = GsmRegistrationState::Roaming;
		return true;
	default:
		return false;
	}
}

bool ParsingHelpers::IsImeiValid(FixedStringBase &imei)
{
	if (imei.length() < 10)
	{
		return false;
	}
	if (imei.length() > 18)
	{
		return false;
	}
	for (int i = 0; i < imei.length(); i++)
	{
		auto c = imei.c_str()[i];
		if (c < '0' || c > '9')
		{
			return false;
		}
	}
	return true;
}
bool ParsingHelpers::ParseIpAddress(FixedStringBase &ipAddress, GsmIp& ip)
{
	DelimParser parser(ipAddress, '.');
	uint8_t octet;
	int n = 0;
	while (n < 4 && parser.NextNum(octet))
	{
		ip._octets[n] = octet;
		n++;
	}

	return n == 4;
}

bool ParsingHelpers::ParseProtocolType(FixedString20& protocolStr, ProtocolType& protocol)
{
	if (protocolStr == F("TCP"))
	{
		protocol = ProtocolType::Tcp;
		return true;
	}
	if (protocolStr == F("UDP"))
	{
		protocol = ProtocolType::Tcp;
		return true;
	}
	return false;
}

bool ParsingHelpers::ParseConnectionState(FixedString20& connectionStateStr, ConnectionState& connectionState)
{
	if (connectionStateStr == F("INITIAL"))
	{
		connectionState = ConnectionState::Initial;
		return true;
	}
	if (connectionStateStr == F("CONNECTING"))
	{
		connectionState = ConnectionState::Connecting;
		return true;
	}
	if (connectionStateStr == F("CONNECTED"))
	{
		connectionState = ConnectionState::Connected;
		return true;
	}
	if (connectionStateStr == F("REMOTE CLOSING"))
	{
		connectionState = ConnectionState::RemoteClosing;
		return true;
	}if (connectionStateStr == F("CLOSING"))
	{
		connectionState = ConnectionState::Closing;
		return true;
	}
	if (connectionStateStr == F("CLOSED"))
	{
		connectionState = ConnectionState::Closed;
		return true;
	}

	return false;
}


struct IpStatusEntry
{
	const __FlashStringHelper *str;
	SimcomIpState status;
};
const  char IpInitalStr[] PROGMEM  = "STATE: IP INITIAL";
const char IpStartStr[]  PROGMEM = "STATE: IP START";
const char IpConfigStr[]  PROGMEM = "STATE: IP CONFIG";
const char IpGporsActStr[]  PROGMEM = "STATE: IP GPRSACT";
const char IpStatusStr[]  PROGMEM = "STATE: IP STATUS";
const char TcpConnectingStr[]  PROGMEM = "STATE: TCP CONNECTING";
const char TcpClosedStr[]  PROGMEM = "STATE: TCP CLOSED";
const char PdpDeactStr[]  PROGMEM = "STATE: PDP DEACT";
const char ConnectOkStr[]  PROGMEM = "STATE: CONNECT OK";
const char IpProcessingStr[]  PROGMEM = "STATE: IP PROCESSING";


#define FPSTR(x) reinterpret_cast<const __FlashStringHelper*>(x)

struct IpStatusEntry IpStatusMap[] =
{
	FPSTR(IpInitalStr), SimcomIpState::IpInitial,
	FPSTR(IpStartStr), SimcomIpState::IpStart,
	FPSTR(IpConfigStr), SimcomIpState::IpConfig,
	FPSTR(IpGporsActStr), SimcomIpState::IpGprsact,
	FPSTR(IpStatusStr), SimcomIpState::IpStatus,
	FPSTR(TcpConnectingStr), SimcomIpState::TcpConnecting,
	FPSTR(TcpClosedStr), SimcomIpState::TcpClosed,
	FPSTR(PdpDeactStr), SimcomIpState::PdpDeact,
	FPSTR(ConnectOkStr), SimcomIpState::ConnectOk,
	FPSTR(IpProcessingStr), SimcomIpState::IpProcessing,
	0,SimcomIpState::Unknown
};

bool ParsingHelpers::ParseIpStatus(const char *str, SimcomIpState &status)
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

bool ParsingHelpers::CheckIfLineContainsGarbage(FixedStringBase & line)
{
	if (line.length() < 3)
	{
		return false;
	}
	int garbageCharacterCount = 0;
	for (int i = 0; i < line.length(); i++)
	{
		if (line[i] > 127)
		{
			garbageCharacterCount++;
		}
	}
	return garbageCharacterCount > 2;
}
