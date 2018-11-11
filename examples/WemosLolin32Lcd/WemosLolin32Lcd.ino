#include <WiFi.h>
#include <SimcomGsmLib.h>
#include <GsmDebugHelpers.h>
#include <OperatorNameHelper.h>
#include <MappingHelpers.h>
#include <SSD1306.h>
#include "Gui.h"
#include "ConnectionDataValidator.h"
#include <Wire.h>

void UpdateBaudRate(int baudRate)
{
	Serial2.flush();
	Serial2.clearWriteError();
	Serial2.end();
	Serial2.begin(baudRate, SERIAL_8N1, 16, 12, false);
}
SimcomGsm gsm(Serial2, UpdateBaudRate);

SSD1306 display(188, 4, 15); // 60 or 188

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
	gsm.Logger().LogAtCommands = false;
	gsm.Logger().OnLog(OnLog);

	Serial.begin(500000);

	gui.init();
	gsm.OnDataReceived(OnDataReceived);
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

	
	if (!gsm.EnsureModemConnected(115200))
	{
		FixedString20 error = "No shield";
		gui.DisplayError(error);
		delay(1000);
		return;
	}

	if (justConnectedToModem)
	{
		justConnectedToModem = false;
		gsm.Cipshut();
	}

	SimState simStatus;
	if (gsm.GetSimStatus(simStatus) == AtResultType::Success)
	{
		if (simStatus != SimState::Ok)
		{
			gui.DisplaySimError(simStatus);
			delay(1000);
			return;
		}
	}

	int16_t signalQuality;
	BatteryStatus batteryInfo;
	FixedString20 operatorName;
	IncomingCallInfo callInfo;
	GsmRegistrationState gsmRegStatus;
	GsmIp ipAddress;
	SimcomIpState ipStatus;

	if (gsm.GetSignalQuality(signalQuality) == AtResultType::Timeout)
	{
		return;
	}
	if (gsm.GetBatteryStatus(batteryInfo) == AtResultType::Timeout)
	{
		return;
	}
	if (OperatorNameHelper::GetRealOperatorName(gsm, operatorName) == AtResultType::Timeout)
	{
		return;
	}
	if (gsm.GetIncomingCall(callInfo) == AtResultType::Timeout)
	{
		return;
	}
	if (gsm.GetIpState(ipStatus) == AtResultType::Timeout)
	{
		return;
	}

	auto getIpResult = gsm.GetIpAddress(ipAddress);
	if (getIpResult == AtResultType::Timeout)
	{
		return;
	}
	bool hasIpAddress = getIpResult == AtResultType::Success;	
	if (gsm.GetRegistrationStatus(gsmRegStatus) == AtResultType::Timeout)
	{
		return;
	}


	if (!hasIpAddress)
	{
		gsm.Cipshut();
	}

	if (gsmRegStatus == GsmRegistrationState::Roaming || gsmRegStatus == GsmRegistrationState::HomeNetwork)
	{
		if (!hasIpAddress)
		{
			gui.Clear();
			display.setFont(ArialMT_Plain_10);

			gsm.SetCipmux(true);
			gsm.SetRxMode(true);
			gsm.SetApn("virgin-internet", "", "");
			display.drawString(0, 0, "Connecting to gprs..");
			display.display();
			delay(400);
			gsm.AttachGprs();
		}
	}
	

	ConnectionInfo info;

	if (hasIpAddress)
	{
		if (gsm.GetConnectionInfo(0, info) == AtResultType::Success)
		{	
			if (info.State == ConnectionState::Closed || info.State == ConnectionState::Initial)
			{
				Serial.printf("Trying to connect...\n");
				receivedBytes = 0;
				connectionValidator.SetJustConnected();
				gsm.BeginConnect(ProtocolType::Tcp, 0, "conti.ml", 12668);
			}
		}
		else
		{
			Serial.println("Connection info failed");
		}
	}

	gui.drawBattery(batteryInfo.Percent, batteryInfo.Voltage);
	gui.drawGsmInfo(signalQuality, gsmRegStatus, operatorName);

	

	if (hasIpAddress)
	{
		gui.DisplayIp(ipAddress);
	}
	gui.DisplayBlinkIndicator();

	if (hasIpAddress)
	{
		ReadDataFromConnection();
	}

	if (hasIpAddress)
	{
		FixedString50 receivedBytesStr;
		receivedBytesStr.appendFormat("received: %d b", receivedBytes);
		display.setColor(OLEDDISPLAY_COLOR::WHITE);

		display.drawString(0, 64 - 22, ConnectionStateToStr(info.State));
		display.drawString(0, 64 - 12, receivedBytesStr.c_str());
	}

	if (gsm.GarbageOnSerialDetected())
	{
		FixedString20 error("UART garbage !!!");
		gui.DrawFramePopup(error, 40, 5);
		Serial.println("Draw garbage detected pupup");
	}

	gui.DisplayIncomingCall(callInfo);

	
	
	display.display();

	gsm.wait(1000);
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
