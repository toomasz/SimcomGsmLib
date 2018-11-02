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
	ConnectionDataValidator()
	{
		prevChar = 255;
	}
	void SetJustConnected()
	{
		prevChar = 255;
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
		if (c - 1 != prevChar && (c != 0 || prevChar != 255))
		{
			FixedString100 error;
			error.appendFormat("Invalid sequence: [%d, %d]\npos=%d\nreceived: %d b", prevChar, c, position, receivedBytes);
			Serial.println(error.c_str());

			SetDataError(error);
		}
		prevChar = c;
	}
};

