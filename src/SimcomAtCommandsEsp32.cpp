#include "SimcomAtCommandsEsp32.h"

#ifdef ESP32

int SimcomAtCommandsEsp32::_txPin = 0;
int SimcomAtCommandsEsp32::_rxPin = 0;
int SimcomAtCommandsEsp32::_dtrPin = 0;

bool SimcomAtCommandsEsp32::_isSerialInitialized = false;
HardwareSerial* SimcomAtCommandsEsp32::_serial = nullptr;

#endif