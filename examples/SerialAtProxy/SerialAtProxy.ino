#include <SimcomAtCommandsEsp32.h>


SimcomAtCommandsEsp32 gsm(Serial2, 16, 14);

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
	Serial.begin(500000);
	Serial.println("Trying to find modem baud rate...");

	Serial2.begin(115200, SERIAL_8N1, 16, 14, false);

	/*auto baudRate = gsm.FindCurrentBaudRate();
	Serial.print("Found baud rate: ");
	Serial.print(baudRate);*/
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
