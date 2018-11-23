#include "GsmLogger.h"

#include <stdarg.h>
#include <stdio.h>
#include <FixedString.h>

GsmLogger::GsmLogger()
{
	_onLog = nullptr;
	LogAtCommands = false;
}
void GsmLogger::OnLog(GsmLogCallback onLog)
{
	_onLog = onLog;
}

void GsmLogger::Log(const __FlashStringHelper* format, ...)
{
	va_list argptr;
	va_start(argptr, format);

	FixedString200 buffer;
	buffer.appendFormatV(format, argptr);
	if (_onLog != nullptr)
	{
		_onLog(buffer.c_str());
	}

	va_end(argptr);
}

void GsmLogger::LogAt(const __FlashStringHelper* format, ...)
{
	if (!LogAtCommands)
	{
		return;
	}
	va_list argptr;
	va_start(argptr, format);

	FixedString200 buffer;
	buffer.appendFormatV(format, argptr);
	if (_onLog != nullptr)
	{
		_onLog(buffer.c_str());
	}

	va_end(argptr);
}