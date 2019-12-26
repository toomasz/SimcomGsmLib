#include "GsmLogger.h"

#include <stdarg.h>
#include <stdio.h>
#include <FixedString.h>

GsmLogger::GsmLogger()
{
	_onLog = nullptr;
}
void GsmLogger::OnLog(GsmLogCallback onLog)
{
	_onLog = onLog;
}

void GsmLogger::Log(const __FlashStringHelper* format, ...)
{
	if (!LogEnabled)
	{
		return;
	}
	va_list argptr;
	va_start(argptr, format);

	FixedString128 buffer;
	buffer.appendFormatV(format, argptr);
	if (_onLog != nullptr)
	{
		_onLog(buffer.c_str(), false);
	}

	va_end(argptr);
}

void GsmLogger::LogAt(const __FlashStringHelper* format, ...)
{
	if (!LogEnabled)
	{
		return;
	}
	if (!LogAtCommands)
	{
		return;
	}
	va_list argptr;
	va_start(argptr, format);

	FixedString128 buffer;
	buffer.appendFormatV(format, argptr);
	if (_onLog != nullptr)
	{
		_onLog(buffer.c_str(), false);
	}

	va_end(argptr);
}

void GsmLogger::Flush()
{
	if (_onLog != nullptr)
	{
		_onLog("", true);
	}
}