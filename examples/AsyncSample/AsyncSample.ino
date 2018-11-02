#include <SimcomGsmLib.h>

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

SimcomGsm gsm(Serial2, UpdateBaudRate);

void setup()
{
	gsm.Logger().OnLog(OnLog);
	gsm.IsAsync = true;
	Serial.begin(500000);
}


// the loop function runs over and over again until power down or reset
void loop() {
  gsm
}
