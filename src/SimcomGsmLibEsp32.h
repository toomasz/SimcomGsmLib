#ifndef _SIMCOM_GSM_LIB_ESP32_H
#define _SIMCOM_GSM_LIB_ESP32_H

#include <HardwareSerial.h>
#include "SimcomGsmLib.h"



class SimcomGsmpEsp32 : public SimcomGsm
{
	static int _txPin;
	static int _rxPin;
	static void UpdateBaudRate(int baudRate)
	{
		Serial2.flush();
		Serial2.end();
		Serial2.begin(baudRate, SERIAL_8N1, _txPin, _rxPin, false);
	}

public:
	SimcomGsmpEsp32(HardwareSerial& serial, int txPin, int rxPin)
		:SimcomGsm(serial, UpdateBaudRate)
	{
		_txPin = txPin;
		_rxPin = rxPin;
	}
};

#endif

