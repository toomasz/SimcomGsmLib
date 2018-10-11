#include "GsmLogger.h"

#include <stdarg.h>
#include <stdio.h>

GsmLogger::GsmLogger()
{
	_onLog = nullptr;
}
void GsmLogger::SetLogCallback(GsmLogCallback onLog)
{
	_onLog = onLog;
}

void GsmLogger::Log_P(const __FlashStringHelper* format, ...)
{
	va_list argptr;
	va_start(argptr, format);

	char logBuffer[200];
	vsnprintf_P(logBuffer, 200, (PGM_P)format, argptr);

	if (_onLog != nullptr)
	{
		_onLog(logBuffer);
	}

	va_end(argptr);
}