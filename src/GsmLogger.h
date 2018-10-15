#ifndef _GSM_LOGGER_H
#define _GSM_LOGGER_H

#include <pgmspace.h>
#include <WString.h>

typedef void(*GsmLogCallback)(const char* logLine);

class GsmLogger
{
	GsmLogCallback _onLog;
public:
	GsmLogger();
	void SetLogCallback(GsmLogCallback onLog);
	void Log_P(const __FlashStringHelper* format, ...);
};

#endif
