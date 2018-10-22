#include <SimcomGsmLib.h>
SimcomGsm gsm(Serial2, UpdateBaudRate);

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
	gsm.SetLogCallback(OnLog);
	Serial.begin(115200);
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

	gsm.GetSignalQuality(signalQuality);
	gsm.GetIncomingCall(callInfo);
	
	Serial.printf("Signal quality: %d\n", signalQuality);
	if (callInfo.HasIncomingCall)
	{
		Serial.printf("Incoming call from %s\n", callInfo.CallerNumber.c_str());
	}
	delay(500);
}
