#include "SimcomGsmLibEsp32.h"

int SimcomGsmpEsp32::_txPin = 0;
int SimcomGsmpEsp32::_rxPin = 0;
bool SimcomGsmpEsp32::_isSerialInitialized = false;
HardwareSerial* SimcomGsmpEsp32::_serial = nullptr;