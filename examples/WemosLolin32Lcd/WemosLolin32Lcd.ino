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
	Serial1.end();	
	Serial1.setRxBufferSize(50000);
	Serial1.begin(baudRate, SERIAL_8N1, 19, 18, false);
}
SimcomGsm gsm(Serial1, UpdateBaudRate);
SSD1306 display(0x3c, 5, 4);
Gui gui(display);
ConnectionDataValidator connectionValidator;

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
	Serial.printf(" #####  Data received: %s\n", data.c_str());
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
	gsm.OnDataReceived(OnDataReceived);
}

void loop()
{
	gui.Clear();

	if (connectionValidator.HasError())
	{
		display.drawString(0, 2, connectionValidator.GetError().c_str());
		display.display();
		delay(500);
		return;
	}

	if (!gsm.EnsureModemConnected(115200))
	{
		display.drawString(0, 2, "No shield");
		display.display();
		delay(200);
		return;
	}

	int16_t signalQuality;
	BatteryStatus batteryInfo;
	FixedString20 operatorName;
	IncomingCallInfo callInfo;
	SimcomIpState ipStatus;
	GsmRegistrationState gsmRegStatus;

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
	if (gsm.GetRegistrationStatus(gsmRegStatus) == AtResultType::Timeout)
	{
		return;
	}

	bool hasCipmux;
	if (gsm.GetCipmux(hasCipmux) == AtResultType::Success)
	{
		if (!hasCipmux)
		{
			Serial.println("Cipmux disabled, attempting to enable");
			if (gsm.SetCipmux(true) == AtResultType::Error)
			{
				Serial.println("Failed to set cipmux");
				gsm.Cipshut();
				gsm.SetCipmux(true);
				
			}
		}
	}

	if (ipStatus == SimcomIpState::PdpDeact)
	{
		gsm.Cipshut();
	}

	if (gsmRegStatus == GsmRegistrationState::Roaming || gsmRegStatus == GsmRegistrationState::HomeNetwork)
	{
		if (ipStatus == SimcomIpState::PdpDeact ||
			ipStatus == SimcomIpState::IpInitial ||
			ipStatus == SimcomIpState::IpGprsact ||
			ipStatus == SimcomIpState::IpStart)
		{
			gui.Clear();
			display.setFont(ArialMT_Plain_10);
			gsm.SetApn("virgin-internet", "", "");
			display.drawString(0, 0, "Connecting to gprs..");
			display.display();
			delay(400);
			gsm.AttachGprs();
		}
	}
	

	GsmIp ipAddress;
	ConnectionInfo info;

	gsm.GetIpAddress(ipAddress);
	if (ipStatus == SimcomIpState::IpStatus || ipStatus == SimcomIpState::IpProcessing)
	{
		if (gsm.GetConnectionInfo(0, info) == AtResultType::Success)
		{
			Serial.printf("Conn info: bearer=%d, ctx=%d,proto=%s endpoint = [%s:%d] state = [%s]\n",
				info.Mux, info.Bearer,
				ProtocolToStr(info.Protocol), info.RemoteAddress.ToString().c_str(),

				info.Port, ConnectionStateToStr(info.State));

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
	gui.DisplayIp(ipAddress);
	gui.DisplayBlinkIndicator();
	gui.DisplayIncomingCall(callInfo);

	
	if (ipStatus == SimcomIpState::IpStatus || ipStatus == SimcomIpState::IpProcessing)
	{
		FixedString50 receivedBytesStr;
		receivedBytesStr.appendFormat("received: %d b", receivedBytes);
		display.setColor(OLEDDISPLAY_COLOR::WHITE);

		display.drawString(0, 64 - 22, ConnectionStateToStr(info.State));
		display.drawString(0, 64 - 12, receivedBytesStr.c_str());
	}
	Serial.println("Display");
	display.display();

	gsm.wait(1000);
}
