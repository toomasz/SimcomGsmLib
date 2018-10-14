#ifndef _MAPPINGHELPERS_h
#define _MAPPINGHELPERS_h

#include "Sim900Constants.h"

GsmNetworkStatus CregToNetworkStatus(uint16_t status);

bool ParseIpStatus(const char *str, SimcomIpState &status);

#endif

