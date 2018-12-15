#include <SimcomAtCommands.h>
#include <SoftwareSerial.h>

SoftwareSerial ss(10,9);

void UpdateBaudRate(uint64_t baudRate)
{
	ss.end();
	ss.begin(baudRate);
}

SimcomAtCommands gsm(ss, UpdateBaudRate);

void OnLog(const char* gsmLog)
{
	Serial.print("[GSM]" );
	Serial.println(gsmLog);
}

void setup()
{
	delay(500);
	gsm.Logger().LogAtCommands = true;
	gsm.Logger().OnLog(OnLog);

	Serial.begin(115200);
	Serial.println("Trying to find modem baud rate...");
	auto baudRate = gsm.FindCurrentBaudRate();
	Serial.print("Found baud rate: ");
	Serial.print(baudRate);
}

void loop()
{
	while (ss.available())
	{
		Serial.write(ss.read());
	}
	while (Serial.available())
	{
		ss.write(Serial.read());
	}
}
