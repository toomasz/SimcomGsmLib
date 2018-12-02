#include <SimcomGsmLib.h>
#include <GsmLibHelpers.h>

SimcomAtCommands gsm(Serial2, UpdateBaudRate);

void UpdateBaudRate(int baudRate)
{
	Serial2.end();
	Serial2.begin(baudRate, SERIAL_8N1, 19, 18, false);
}
void OnLog(const char* gsmLog)
{
	Serial.print("[GSM]");
	Serial.println(gsmLog);
}
void setup()
{
	gsm.Logger().OnLog(OnLog);
	Serial.begin(500000);

}

void loop()
{
	if (!gsm.EnsureModemConnected(115200))
	{
		Serial.println("No modem found");
		delay(500);
		return;
	}
	
	int16_t signalQuality;
	IncomingCallInfo callInfo;
	GsmRegistrationState registrationStatus;

	gsm.GetSignalQuality(signalQuality);
	gsm.GetIncomingCall(callInfo);
	if (gsm.GetRegistrationStatus(registrationStatus) == AtResultType::Success)
	{
		Serial.printf("reg status: %s\n", RegStatusToStr(registrationStatus));
	}
	else
	{
		Serial.println("Failed to get reg status");
	}
	Serial.printf("Signal quality: %d\n", signalQuality);
	if (callInfo.HasIncomingCall)
	{
		Serial.printf("Incoming call from %s\n", callInfo.CallerNumber.c_str());
	}
	GsmIp ip;
	gsm.GetIpAddress(ip);
	Serial.printf("IP: %s\n", ip.ToString().c_str());
	delay(500);
}
