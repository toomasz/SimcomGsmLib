#ifndef _MAPPINGHELPERS_h
#define _MAPPINGHELPERS_h

#include "SimcomGsmTypes.h"
#include <stdint.h>
#include <WString.h>

const __FlashStringHelper* RegStatusToStr(GsmRegistrationState state);
const __FlashStringHelper* ProtocolToStr(ProtocolType protocol);
const __FlashStringHelper* ConnectionStateToStr(ConnectionState state);

#endif

