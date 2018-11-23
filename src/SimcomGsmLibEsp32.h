#ifndef _SIMCOM_GSM_LIB_ESP32_H
#define _SIMCOM_GSM_LIB_ESP32_H

#include <HardwareSerial.h>
#include "SimcomGsmLib.h"

class SimcomGsmpEsp32 : public SimcomGsm
{
	static int _txPin;
	static int _rxPin;
	static HardwareSerial* _serial;
	static bool _isSerialInitialized;
	static void UpdateBaudRate(int baudRate)
	{
		if (_isSerialInitialized)
		{
			_serial->flush();
			_serial->clearWriteError();
			_serial->end();
			delay(10);
		}
		_serial->begin(baudRate, SERIAL_8N1, _txPin, _rxPin, false);
		_isSerialInitialized = true;
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

