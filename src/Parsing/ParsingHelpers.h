#ifndef  _PARSING_HELPERS_H
#define _PARSING_HELPERS_H

#include <FixedString.h>
#include "SimcomGsmTypes.h"
#include "DelimParser.h"
class ParsingHelpers
{
public:
	static bool ParseRegistrationStatus(uint16_t status, GsmRegistrationState& gsmStatus);
	static bool IsImeiValid(FixedStringBase &imei);
	static bool ParseIpAddress(FixedStringBase &ipAddress, GsmIp& ip);
	static bool ParseProtocolType(FixedString16& protocolStr, ProtocolType& protocol);
	static bool ParseConnectionState(FixedString16& connectionStateStr, ConnectionState& connectionState);
	static bool ParseIpStatus(const char *str, SimcomIpState &status);
	static bool CheckIfLineContainsGarbage(FixedStringBase &line);
	static bool ParseSocketStatusLine(DelimParser& parser, ConnectionInfo& connectionInfo, bool allowNullBearer = false);
};

#endif
