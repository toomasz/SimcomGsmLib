#include <SimcomAtCommands.h>
#include <GsmLibHelpers.h>
#include <OperatorNameHelper.h>
#include <SSD1306.h>
#include "Gui.h"
#include "ConnectionDataValidator.h"
#include <Wire.h>
#include <GsmModule.h>
#include <SimcomAtCommandsEsp32.h>

SimcomAtCommandsEsp32 gsmAt(Serial1, 16, 14);
GsmModule gsm(gsmAt);

SSD1306 display(0x3c, 5, 4);
Gui gui(display);

ConnectionDataValidator connectionValidator;
GsmAsyncSocket *socket = nullptr;

void OnSocketDataReceived(void* ctx, FixedStringBase& data)
{
	if (connectionValidator.HasError())
	{
		return;
	}
	FixedString200 dataStr;
	BinaryToString(data, dataStr);
	Serial.printf("Received %d bytes: '%s'\n", data.length(), dataStr.c_str());
	connectionValidator.ValidateIncomingData(data, socket);	
}

void setup()
{	
	//gsmAt.Logger().LogAtCommands = true;
	Serial.begin(500000);

	gsmAt.Logger().OnLog([](const char *logEntry) 
	{
		Serial.printf("%u8 [GSM]", millis());
		Serial.println(logEntry);
	});

	gui.init();

	socket = gsm.CreateSocket(0, ProtocolType::Tcp);

	socket->OnSocketEvent(nullptr, [](void*ctx, SocketEventType eventType) 
	{
		Serial.printf("Socket event: %s\n", SocketEventTypeToStr(eventType));
	});
	socket->OnDataRecieved(nullptr, OnSocketDataReceived);
}


void loop()
{
	gui.Clear();
	if (gsm.GarbageOnSerialDetected())
	{
		gui.DisplayError("UART garbage !!!");
		delay(500);
		return;
	}
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
			socket->BeginConnect("conti.ml", 12668);
		}

		if (socket->IsConnected())
		{
			if (socket->GetSentBytes() == 0)
			{
				connectionValidator.NotifyConnected();
				socket->Send("1", 1);
			}
			static uint64_t lastDataSend = 0;
			if (millis() - lastDataSend > 10000)
			{
				lastDataSend = millis();
				SendPacket();
			}
		}

		gui.DisplaySocketState(socket);
	}

	
	gui.Display();
	Serial.println("\n       ######       \n");
	gsm.Wait(500);
}
void SendPacket()
{
	FixedString50 data;
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