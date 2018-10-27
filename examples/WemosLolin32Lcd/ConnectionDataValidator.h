#pragma once

#include <FixedString.h>

class ConnectionDataValidator
{
	FixedString100 dataError;
	bool hasDataError = false;
	bool justConnected = true;
	char prevChar;
	void SetDataError(FixedString100&error)
	{
		hasDataError = true;
		dataError = error;
	}
public:
	void SetJustConnected()
	{
		justConnected = true;
	}
	FixedString100 GetError()
	{
		return dataError;
	}
	bool HasError()
	{
		return hasDataError;
	}
	
	void ValidateIncomingByte(char c, int position, int receivedBytes)
	{
		if (justConnected)
		{
			prevChar = c;
			justConnected = false;
			return;
		}
		if (c - 1 != prevChar && (c != '0' || prevChar != ':'))
		{
			FixedString100 error;
			error.appendFormat("Invalid sequence: [%c, %c]\npos=%d\nreceived: %d b", prevChar, c, position, receivedBytes);
			Serial.println(error.c_str());

			SetDataError(error);
		}
		prevChar = c;
	}
};

