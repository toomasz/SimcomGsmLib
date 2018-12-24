#pragma once

#include <FixedString.h>
#include "Network\GsmAsyncSocket.h"

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
	int n;
public:

	void GenerateData(FixedStringBase &targetStr)
	{
		for (int i = 0; i < targetStr.capacity(); i++)
		{
			targetStr.append(n);
			n++;
		}
	}
	void NotifyDataSent(uint16_t dataLength, uint16_t sentBytes)
	{
		if (dataLength > sentBytes)
		{
			n -= dataLength - sentBytes;
		}
	}
	ConnectionDataValidator()
	{
		n = 0;
		prevChar = 255;
	}
	void NotifyConnected()
	{
		n = 0;
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

	void ValidateIncomingData(FixedStringBase& data, GsmAsyncSocket *socket)
	{
		for (int i = 0; i < data.length(); i++)
		{
			ValidateIncomingByte(data[i], i, socket->GetReceivedBytes());
		}
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

