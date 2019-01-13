#ifndef _GSM_LOGGER_H
#define _GSM_LOGGER_H

#include <pgmspace.h>
#include <WString.h>

typedef void(*GsmLogCallback)(const char* logLine, bool flush);

class GsmLogger
{
	GsmLogCallback _onLog;
public:
	bool LogEnabled = true;
	bool LogAtCommands = false;
	GsmLogger();	 
	void OnLog(GsmLogCallback onLog);
	void Log(const __FlashStringHelper * format, ...);
	void LogAt(const __FlashStringHelper* format, ...);
	void Flush();
};

#endif
