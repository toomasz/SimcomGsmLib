# SimcomGsmLib

Library for simcom modules (tested only on Sim800/Sim900)

## Features:
 - Not using arduino Strings, it uses ArduinoFixedString library that is safe equivalent of char arrays
 - Automatic Serial baud rate negotiation
 - Basic info: Signal quality, Operator Name, Battery Status, etc
 - Incoming call info
 - Networking - async TCP/UDP sockets
 
## Installation:
 1. Download repo to Arduino libraries directory(on windows - c:\Users\USERNAME\Documents\Arduino\libraries)
 2. Download https://github.com/toomasz/ArduinoFixedString and extract it to Arduino libraries directory
 3. Try sample sketch:
 ```c++
 #include <SimcomGsmLib.h>
SimcomGsm gsm(Serial2, UpdateBaudRate);

void UpdateBaudRate(int baudRate)
{
	Serial2.end();
	Serial2.begin(baudRate, SERIAL_8N1, 19, 18, false);
}

void setup()
{
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
```

 


