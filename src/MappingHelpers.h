#ifndef _MAPPINGHELPERS_h
#define _MAPPINGHELPERS_h

#include "SimcomGsmTypes.h"
#include <stdint.h>
#include <WString.h>

GsmNetworkStatus CregToNetworkStatus(uint16_t status);
bool ParseIpStatus(const char *str, SimcomIpState &status);
bool ParseProtocolType(FixedString20& protocolStr, ProtocolType& protocol);
bool ParseConnectionState(FixedString20& connectionStateStr, ConnectionState& connectionState);

const __FlashStringHelper* ProtocolToStr(ProtocolType protocol);
#endif

