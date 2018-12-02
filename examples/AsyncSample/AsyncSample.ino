#include <SimcomGsmLib.h>

#include "Gui.h"

Gui ui(188, 4, 15);

void UpdateBaudRate(int baudRate)
{
	Serial2.flush();
	Serial2.end();
	Serial2.begin(baudRate, SERIAL_8N1, 14, 12, false);
}
void OnLog(const char* gsmLog)
{
	Serial.print("[GSM]");
	Serial.println(gsmLog);
}

SimcomAtCommands gsm(Serial2, UpdateBaudRate);

void setup()
{
	gsm.Logger().OnLog(OnLog);
	gsm.Logger().LogAtCommands = true;
	gsm.IsAsync = true;
	Serial.begin(500000);

	ui.init();
}


// the loop function runs over and over again until power down or reset
void loop() 
{
	gsm.EnsureModemConnected(115200);
	ui.Clear();
	FixedString20 msg = "Test";
	ui.DrawFramePopup(msg);
	ui.Draw();
	gsm.wait(1000);
}
