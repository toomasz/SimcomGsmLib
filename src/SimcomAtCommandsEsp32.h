#ifndef _SIMCOM_GSM_LIB_ESP32_H
#define _SIMCOM_GSM_LIB_ESP32_H

#ifdef  ESP32

#include <HardwareSerial.h>
#include "SimcomAtCommands.h"

class SimcomAtCommandsEsp32 : public SimcomAtCommands
{
	static int _txPin;
	static int _rxPin;
	static int _dtrPin;
	static HardwareSerial* _serial;
	static bool _isSerialInitialized;
	static void UpdateBaudRate(uint64_t baudRate)
	{
		if (_isSerialInitialized)
		{
			_serial->updateBaudRate(baudRate);
			return;
		}
		
		_serial->begin(baudRate, SERIAL_8N1, _txPin, _rxPin, false);
		_isSerialInitialized = true;		
	}

	static bool SetDtr(bool isHigh)
	{
		if (_dtrPin == -1)
		{
			return false;
		}	
		digitalWrite(_dtrPin, isHigh);
		return true;
	}
	static void LightSleep(uint64_t millis)
	{
		esp_sleep_enable_timer_wakeup(1000 * millis);
		esp_light_sleep_start();
	}
public:
	SimcomAtCommandsEsp32(HardwareSerial& serial, int txPin, int rxPin, int dtrPin = -1)
		:SimcomAtCommands(serial, UpdateBaudRate, SetDtr, LightSleep)
	{
		Serial.println("SimcomAtCommandsEsp32::SimcomAtCommandsEsp32");		
		_serial = &serial;
		_dtrPin = dtrPin;
		if (_dtrPin != -1)
		{
			pinMode(_dtrPin, OUTPUT_OPEN_DRAIN);
		}
		_txPin = txPin;
		_rxPin = rxPin;
	}
};

#endif //  ESP32

#endif

