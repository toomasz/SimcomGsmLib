#include <SimcomAtCommandsEsp32.h>


SimcomAtCommandsEsp32 gsm(Serial2, 33, 32);

void OnLog(const char* gsmLog, bool _)
{
	Serial.print("[GSM]" );
	Serial.println(gsmLog);
}

void setup()
{
	delay(500);
	gsm.Logger().LogAtCommands = true;
	gsm.Logger().OnLog(OnLog);
	Serial.begin(500000);
	Serial.println("Trying to find modem baud rate...");


	auto baudRate = gsm.FindCurrentBaudRate();
	Serial.printf("Found baud rate: %d\n", baudRate);
}

void loop()
{
	while (Serial2.available())
	{
		Serial.write(Serial2.read());
	}
	while (Serial.available())
	{
		Serial2.write(Serial.read());
	}
}
