#include <SimcomAtCommands.h>
#include <GsmLibHelpers.h>
#include <OperatorNameHelper.h>
#include <SSD1306.h>
#include "Gui.h"
#include "ConnectionDataValidator.h"
#include <Wire.h>
#include <GsmModule.h>
#include <SimcomAtCommandsEsp32.h>
#include <FixedString.h>

SimcomAtCommandsEsp32 gsmAt(Serial1, 16, 14, 25);
GsmModule gsm(gsmAt);

Gui gui(0x3c, 5, 4);

ConnectionDataValidator connectionValidator;
GsmAsyncSocket *socket = nullptr;

void OnSocketDataReceived(void* ctx, FixedStringBase& data)
{
	if (connectionValidator.HasError())
	{
		return;
	}
	PrintIncomingData(data);
	connectionValidator.ValidateIncomingData(data, socket);	
}

void PrintIncomingData(FixedStringBase& data)
{
	FixedString200 dataStr;
	BinaryToString(data, dataStr);
	Serial.printf("Received %d bytes: '%s'\n", data.length(), dataStr.c_str());
}

void setup()
{	
	gsmAt.Logger().LogAtCommands = true;
	Serial.begin(500000);

	gsm.OnLog([](const char *logEntry)
	{
		Serial.printf("%u8 [GSM]", millis());
		Serial.println(logEntry);
	});
	gsm.SleepEnabled = true;
	gsm.BaudRate = 460800;
	gsm.ApnName = "virgin-internet";
	gsm.TickInterval = 300;

	gui.init();

	socket = gsm.CreateSocket(0, ProtocolType::Tcp);

	socket->OnSocketEvent(nullptr, [](void*ctx, SocketEventType eventType) 
	{
		if (eventType == SocketEventType::ConnectSuccess)
		{
			connectionValidator.NotifyConnected();
			socket->Send("1", 1);
		}
		Serial.printf("Socket event: %s\n", SocketEventTypeToStr(eventType));
	});
	
	socket->OnDataRecieved(nullptr, OnSocketDataReceived);
}


void loop()
{
	gui.Clear();	
	if (connectionValidator.HasError())
	{
		gui.DisplayError(connectionValidator.GetError());
		delay(500);
		return;
	}
	gsm.Loop();

	const auto state = gsm.GetState();
	gui.DisplayGsmState(gsm);

	if (socket->IsNetworkAvailable())
	{
		if (socket->IsClosed())
		{
			static uint64_t lastConnectTime =0;
			if (millis() - lastConnectTime > 5000)
			{
				lastConnectTime = millis();
				socket->BeginConnect("conti.ml", 12668);
			}
		}

		if (socket->IsConnected())
		{			
			static uint64_t lastDataSend = 0;
			if (millis() - lastDataSend > 20000)
			{
				lastDataSend = millis();
				SendPacket();
			}
		}

		gui.DisplaySocketState(socket);
	}

	
	gui.Display();
	//Serial.println("\n       ######       \n");
}
void SendPacket()
{
	FixedString20 data;
	connectionValidator.GenerateData(data);
	uint16_t sentBytes = socket->Send(data);
	if(sentBytes == -1)
	{
		Serial.printf("Failed to send data");
		return;
	}
	Serial.printf("Success sent %d bytes\n", sentBytes);
	connectionValidator.NotifyDataSent(data.length(), sentBytes);
}