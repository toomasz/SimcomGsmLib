#ifndef _GSM_LOGGER_H
#define _GSM_LOGGER_H

#include <pgmspace.h>
#include <WString.h>

typedef void(*GsmLogCallback)(const char* logLine);

class GsmLogger
{
	GsmLogCallback _onLog;
public:
	bool LogAtCommands;
	GsmLogger();	 
	void OnLog(GsmLogCallback onLog);
	void Log(const __FlashStringHelper * format, ...);
	void LogAt(const __FlashStringHelper* format, ...);
};

#endif
