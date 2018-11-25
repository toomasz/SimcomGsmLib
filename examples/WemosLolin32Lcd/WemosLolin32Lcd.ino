#include <WiFi.h>
#include <SimcomGsmLib.h>
#include <GsmDebugHelpers.h>
#include <OperatorNameHelper.h>
#include <MappingHelpers.h>
#include <SSD1306.h>
#include "Gui.h"
#include "ConnectionDataValidator.h"
#include <Wire.h>
#include <GsmTcpip.h>

#include <SimcomGsmLibEsp32.h>

SimcomGsmpEsp32 gsm(Serial1, 16, 12);

GsmTcpip tcp(gsm);

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
	gsm.Logger().LogAtCommands = true;
	gsm.Logger().OnLog(OnLog);

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

	tcp.Loop();

	auto state = tcp.GetState();

	if (state == GsmState::NoShield)
	{
		FixedString20 error = "No shield";
		gui.DisplayError(error);
		return;
	}

	if (state == GsmState::SimError)
	{
		gui.DisplaySimError(tcp.simStatus);
		delay(1000);
		return;
	}

	gui.drawBattery(tcp.batteryInfo.Percent, tcp.batteryInfo.Voltage);
	gui.drawGsmInfo(tcp.signalQuality, tcp.gsmRegStatus, tcp.operatorName);
	gui.DisplayBlinkIndicator();
	
	if (state == GsmState::ConnectingToGprs)
	{
		gui.lcd_label(Font::F10, 0, 32, F("Connecting to gprs.."));
	}
	if (state == GsmState::Initializing)
	{
		gui.lcd_label(Font::F10, 0, 32, F("Initializing modem..."));

	}


	if (state == GsmState::ConnectedToGprs)
	{
		ConnectionInfo connection;

		auto connectionInfoResult = gsm.GetConnectionInfo(0, connection);
		if(connectionInfoResult == AtResultType::Success)
		{
			static bool justConnected = false;
			if (connection.State == ConnectionState::Closed || connection.State == ConnectionState::Initial)
			{
				Serial.printf("Trying to connect...\n");
				receivedBytes = 0;
				connectionValidator.SetJustConnected();
				gsm.BeginConnect(ProtocolType::Tcp, 0, "conti.ml", 12668);
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
					if(gsm.Send(0, dataToSend) != AtResultType::Success)
					{
						Serial.println("Failed to send data");
					}
				}

				FixedString20 data;
				for(int i=0; i < data.capacity(); i++)
				{				
					data.append(n);
					n ++;
				}
				gsm.Send(0, data);

				ReadDataFromConnection();
			}
			gui.lcd_label(Font::F10, 0, 64 - 22, F("%s"), ConnectionStateToStr(connection.State));
			gui.lcd_label(Font::F10, 0, 64 - 12, F("received: %d b"), receivedBytes);
		}
		else
		{
			Serial.println("Failed to get conn info");
		}

		gui.DisplayIp(tcp.ipAddress);	
	}

	if (gsm.GarbageOnSerialDetected())
	{
		FixedString20 error("UART garbage !!!");
		gui.DrawFramePopup(error, 40, 5);
		Serial.println("Draw garbage detected pupup");
	}

	gui.DisplayIncomingCall(tcp.callInfo);
	
	display.display();

	gsm.wait(500);
}

void ReadDataFromConnection()
{
	FixedString200 buffer;
	while (gsm.Read(0, buffer) == AtResultType::Success)
	{
		if (buffer.length() == 0)
		{
			return;
		}
		OnDataReceived(0, buffer);
		buffer.clear();
	}
}
