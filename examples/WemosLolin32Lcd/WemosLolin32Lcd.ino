
//#include <MappingHelpers.h>
#include <WiFi.h>
#include <SimcomGsmLib.h>
#include <GsmDebugHelpers.h>

#include <SSD1306.h>

void UpdateBaudRate(int baudRate)
{
	Serial2.end();
	Serial2.begin(baudRate, SERIAL_8N1, 19, 18, false);
}
SimcomGsm gsm(Serial2, UpdateBaudRate);
SSD1306 display(0x3c, 5, 4);

void OnLog(const char* gsmLog)
{
	Serial.print("[GSM]");
	Serial.println(gsmLog);
}

void setup()
{
	WiFi.mode(WIFI_OFF);

	Serial.begin(115200);
	display.init();

	display.setFont(ArialMT_Plain_16);
	display.clear();
	gsm.SetLogCallback(OnLog);
}

const char* RegistrationStatusToStr(GsmNetworkStatus regStatus)
{
	switch (regStatus)
	{
	case GsmNetworkStatus::SearchingForNetwork:
		return "Net Search...";
	case GsmNetworkStatus::HomeNetwork:
		return "Home net";
	case GsmNetworkStatus::RegistrationDenied:
		return "Reg denied";
	case GsmNetworkStatus::RegistrationUnknown:
		return "Reg unknown";
	case GsmNetworkStatus::Roaming:
		return "Roaming";
	default:
		break;
	}
	return "";
}

char* gsmNetworks[][2] =
{
	{"26001", "Plus"},
	{"26002", "T-Mobile"},
	{"26003", "Orange"},
	{"26006", "Play"},

	{ nullptr, nullptr }
};

const char *GetNetworkName(const char* networkName)
{
	int i = 0;
	auto gsmNetwork = gsmNetworks[0];
	while (gsmNetwork[0] != nullptr)
	{
		if (strcmp(networkName, gsmNetwork[0]) == 0)
		{
			return gsmNetwork[1];
		}
		gsmNetwork++;
	}
	return networkName;
}

void lcd_label(int y, int heigth, int fontSize, const __FlashStringHelper* format, ...)
{
	va_list argptr;
	va_start(argptr, format);

	if (fontSize == 16)
	{
		display.setFont(ArialMT_Plain_16);
	}
	if (fontSize == 10)
	{
		display.setFont(ArialMT_Plain_10);
	}

	char buffer[200];
	vsnprintf_P(buffer, 200, (PGM_P)format, argptr);
	display.setColor(OLEDDISPLAY_COLOR::BLACK);
	display.fillRect(0, y, 128, heigth);
	display.setColor(OLEDDISPLAY_COLOR::WHITE);
	display.drawString(0, y, buffer);
	va_end(argptr);
}
void lcd_label(int x, int y, const __FlashStringHelper* format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	char buffer[200];
	vsnprintf_P(buffer, 200, (PGM_P)format, argptr);
	display.drawString(x, y, buffer);
	va_end(argptr);
}

void loop()
{
	if (!gsm.EnsureModemConnected(460800))
	{
		display.drawString(0, 2, "No shield");
		display.display();
		delay(200);
		return;
	}
	int16_t signalQuality;
	if (gsm.GetSignalQuality(signalQuality) == AtResultType::Timeout)
	{
		return;
	}
	BatteryStatus batteryInfo;
	if (gsm.GetBatteryStatus(batteryInfo) == AtResultType::Timeout)
	{
		return;
	}
	FixedString20 operatorName;
	if (gsm.GetOperatorName(operatorName, true) == AtResultType::Timeout)
	{
		return;
	}
	IncomingCallInfo callInfo;
	if (gsm.GetIncomingCall(callInfo) == AtResultType::Timeout)
	{
		return;
	}

	SimcomIpState ipStatus;
	gsm.GetIpState(ipStatus);


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
				if (gsm.SetCipmux(true) == AtResultType::Success)
				{
					Serial.println("cipmux set to 1");
				}
				else
				{
					Serial.println("failed to set cipmux to 1");

				}
			}
		}
		Serial.printf("CIPMUX = %d\n", hasCipmux);
	}

	if (ipStatus == SimcomIpState::PdpDeact)
	{
		gsm.Cipshut();
	}

	if (ipStatus == SimcomIpState::PdpDeact ||
		ipStatus == SimcomIpState::IpInitial ||
		ipStatus == SimcomIpState::IpGprsact ||
		ipStatus == SimcomIpState::IpStart)
	{
		gsm.SetApn("virgin-internet", "", "");
		display.clear();
		display.drawString(0, 0, "Connecting to gprs..");
		gsm.AttachGprs();
	}
	FixedString20 ipAddress;
	gsm.GetIpAddress(ipAddress);

	GsmNetworkStatus gsmRegStatus;
	auto registrationStatus = gsm.GetRegistrationStatus(gsmRegStatus);

	lcd_label(0, 18, 16, F("batt: %d%%, %.2f V"), batteryInfo.Percent, batteryInfo.Voltage);
	lcd_label(18, 18, 16, F("GSM: %d CSQ"), signalQuality);
	lcd_label(34, 13, 10, F("%s [%s]"), RegistrationStatusToStr(gsmRegStatus), ipAddress.c_str());
	lcd_label(47, 10, 10, F("Network: %s"), GetNetworkName(operatorName.c_str()));


	static bool rectState = false;
	rectState = !rectState;
	if (rectState)
	{
		display.setColor(OLEDDISPLAY_COLOR::WHITE);
	}
	else
	{
		display.setColor(OLEDDISPLAY_COLOR::BLACK);
	}

	display.fillRect(128 - 10, 64 - 10, 10, 10);
	display.setColor(OLEDDISPLAY_COLOR::WHITE);

	if (callInfo.HasAtiveCall)
	{
		display.setColor(OLEDDISPLAY_COLOR::WHITE);
		display.drawRect(5, 10, 128 - 10, 45);
		display.setColor(OLEDDISPLAY_COLOR::BLACK);
		display.fillRect(6, 11, 128 - 10 - 2, 45 - 2);
		display.setColor(OLEDDISPLAY_COLOR::WHITE);
		display.setFont(ArialMT_Plain_16);
		lcd_label(10, 14, F("Calling: "));
		lcd_label(10, 32, F("%s"), callInfo.CallerNumber.c_str());
	}



	display.display();

	delay(200);
}
