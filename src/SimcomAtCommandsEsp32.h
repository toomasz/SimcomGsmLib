#ifndef _SIMCOM_GSM_LIB_ESP32_H
#define _SIMCOM_GSM_LIB_ESP32_H

#ifdef  ESP32

#include <HardwareSerial.h>
#include "SimcomAtCommands.h"



class SimcomAtCommandsEsp32 : public SimcomAtCommands
{
	static int _txPin;
	static int _rxPin;
	static HardwareSerial* _serial;
	static bool _isSerialInitialized;
	static void UpdateBaudRate(uint64_t baudRate)
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
	SimcomAtCommandsEsp32(HardwareSerial& serial, int txPin, int rxPin)
		:SimcomAtCommands(serial, UpdateBaudRate)
	{
		_serial = &serial;
		_txPin = txPin;
		_rxPin = rxPin;
	}
};

#endif //  ESP32

#endif

