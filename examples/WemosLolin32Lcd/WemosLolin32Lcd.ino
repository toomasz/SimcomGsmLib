#include <WiFi.h>
#include <SimcomAtCommands.h>
#include <GsmLibHelpers.h>
#include <OperatorNameHelper.h>
#include <SSD1306.h>
#include "Gui.h"
#include "ConnectionDataValidator.h"
#include <Wire.h>
#include <GsmModule.h>

#include <SimcomAtCommandsEsp32.h>

SimcomAtCommandsEsp32 gsmAt(Serial1, 16, 12);
GsmModule gsm(gsmAt);

SSD1306 display(188, 4, 15);
Gui gui(display);

ConnectionDataValidator connectionValidator;
bool justConnectedToModem = true;
void OnLog(const char* gsmLog)
{
	Serial.print("[GSM]");
	Serial.println(gsmLog);
}
int receivedBytes = 0;

void OnDataReceived(uint8_t mux, FixedStringBase &data)
{
	if (connectionValidator.HasError())
	{
		return;
	}
	receivedBytes += data.length();
	Serial.printf("Received %d bytes\n", data.length());
	for (int i = 0; i < data.length(); i++)
	{
		connectionValidator.ValidateIncomingByte(data[i], i, receivedBytes);
	}
}

void setup()
{	
	gsmAt.Logger().LogAtCommands = false;
	gsmAt.Logger().OnLog(OnLog);

	Serial.begin(500000);

	gui.init();
}

void loop()
{
	gui.Clear();

	if (connectionValidator.HasError())
	{
		auto error = connectionValidator.GetError();
		gui.DisplayError(error);
		delay(500);
		return;
	}

	gsm.Loop();

	const auto state = gsm.GetState();
	gui.DisplayGsmState(gsm);

	if (state == GsmState::ConnectedToGprs)
	{
		ConnectionInfo connection;

		auto connectionInfoResult = gsmAt.GetConnectionInfo(0, connection);
		if(connectionInfoResult == AtResultType::Success)
		{
			static bool justConnected = false;
			if (connection.State == ConnectionState::Closed || connection.State == ConnectionState::Initial)
			{
				receivedBytes = 0;
				connectionValidator.SetJustConnected();
				gsmAt.BeginConnect(ProtocolType::Tcp, 0, "conti.ml", 12668);
				justConnected = true;
			}

			if(connection.State == ConnectionState::Connected)
			{
				static uint8_t n = 0;

				if (justConnected)
				{
					n = 0;
					justConnected = false;
					FixedString10 dataToSend = "1";
					uint16_t sentBytes = 0;
					if(gsmAt.Send(0, dataToSend, sentBytes) != AtResultType::Success)
					{
						Serial.println("Failed to send data");
					}
				}

				FixedString50 data;
				for(int i=0; i < data.capacity(); i++)
				{				
					data.append(n);
					n++;
				}
				uint16_t sentBytes = 0;
				if(gsmAt.Send(0, data, sentBytes) == AtResultType::Success)
				{
					Serial.printf("Success sent %d bytes\n", sentBytes);
				}

				if(data.length() > sentBytes)
				{
					n -= data.length() - sentBytes;
				}

				ReadDataFromConnection();
			}
			gui.lcd_label(Font::F10, 0, 64 - 22, F("%s"), ConnectionStateToStr(connection.State));
			gui.lcd_label(Font::F10, 0, 64 - 12, F("received: %d b"), receivedBytes);
		}
		else
		{
			Serial.println("Failed to get conn info");
		}
	}	
	
	display.display();
	gsm.Wait(1000);
}

void ReadDataFromConnection()
{
	FixedString20 buffer;
	while (gsmAt.Read(0, buffer) == AtResultType::Success)
	{
		if (buffer.length() == 0)
		{
			return;
		}
		OnDataReceived(0, buffer);
		buffer.clear();
	}
}
