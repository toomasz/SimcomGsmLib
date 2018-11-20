#include <SimcomGsmLib.h>

void UpdateBaudRate(int baudRate)
{
	Serial2.end();
	Serial2.begin(baudRate, SERIAL_8N1, 16, 12, false);
}

SimcomGsm gsm(Serial2, UpdateBaudRate);

void setup()
{
	Serial.begin(500000);
	delay(500);
	Serial.println("Trying to find modem baud rate...");
	auto baudRate = gsm.FindCurrentBaudRate();
	Serial.printf("Found baud rate: %d\nPlease use AT commands!\n", baudRate);
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
