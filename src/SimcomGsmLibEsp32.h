#ifndef _SIMCOM_GSM_LIB_ESP32_H
#define _SIMCOM_GSM_LIB_ESP32_H

#include <HardwareSerial.h>
#include "SimcomGsmLib.h"



class SimcomGsmpEsp32 : public SimcomGsm
{
	static int _txPin;
	static int _rxPin;
	static HardwareSerial* _serial;
	static void UpdateBaudRate(int baudRate)
	{
		_serial->flush();
		_serial->end();
		_serial->begin(baudRate, SERIAL_8N1, _txPin, _rxPin, false);
	}

public:
	SimcomGsmpEsp32(HardwareSerial& serial, int txPin, int rxPin)
		:SimcomGsm(serial, UpdateBaudRate)
	{
		_serial = &serial;
		_txPin = txPin;
		_rxPin = rxPin;
	}
};

#endif

