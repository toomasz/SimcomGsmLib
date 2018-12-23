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

bool justConnectedToModem = true;
void OnLog(const char* gsmLog)
{
	Serial.printf("%u8 [GSM]", millis());
	Serial.println(gsmLog);
}

void OnSocketEvent(void*ctx, SocketEventType eventType)
{
	Serial.printf("Socket event: %s\n", SocketEventTypeToStr(eventType));	
}

void OnSocketDataReceived(void* ctx, FixedStringBase& data)
{
	if (connectionValidator.HasError())
	{
		return;
	}
	FixedString200 dataStr;
	BinaryToString(data, dataStr);
	Serial.printf("Received %d bytes: '%s'\n", data.length(), dataStr.c_str());
	for (int i = 0; i < data.length(); i++)
	{
		connectionValidator.ValidateIncomingByte(data[i], i, socket->GetReceivedBytes());
	}
}

void setup()
{	
	//gsmAt.Logger().LogAtCommands = true;
	gsmAt.Logger().OnLog(OnLog);

	Serial.begin(500000);
	gui.init();

	socket = gsm.CreateSocket(0, ProtocolType::Tcp);
	socket->OnSocketEvent(nullptr, OnSocketEvent);
	socket->OnDataRecieved(nullptr, OnSocketDataReceived);
}


void loop()
{
	gui.Clear();
	if (gsm.GarbageOnSerialDetected())
	{
		FixedString20 error("UART garbage !!!");
		gui.DrawFramePopup(error, 40, 5);
		gui.Display();
		delay(500);
		return;
	}
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
		gui.lcd_label(Font::F10, 0, 64 - 22, F("%s"), SocketStateToStr(socket->GetState()));
		gui.lcd_label(Font::F10, 0, 64 - 12, F("r/s: %llu b/%llu b"), socket->GetReceivedBytes(), socket->GetSentBytes());
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
	}
	Serial.printf("Success sent %d bytes\n", sentBytes);
	connectionValidator.NotifyDataSent(data.length(), sentBytes);
}