#ifndef _SIMCOM_AT_COMMANDS_H
#define _SIMCOM_AT_COMMANDS_H

#include <Stream.h>
#include <Arduino.h>
#include <stdio.h>
#include <stdarg.h>
#include "Parsing/SequenceDetector.h"
#include "Parsing/SimcomResponseParser.h"
#include "Parsing/ParserContext.h"
#include "GsmLogger.h"
#include "SimcomGsmTypes.h"
#include <pgmspace.h>

class S900Socket;

class SimcomAtCommands
{
private:
		Stream &_serial;
		int _currentBaudRate;
		GsmLogger _logger;
		SimcomResponseParser _parser;
		UpdateBaudRateCallback _updateBaudRateCallback;
		ParserContext _parserContext;
		// buffer for incoming data, used because fucking +++ needs 1000ms wait before issuing
		FixedString50 _currentCommand;
		void SendAt_P(AtCommand commandType, const __FlashStringHelper *command, ...);
		AtResultType PopCommandResult(int timeout);
		AtResultType PopCommandResult();		
public:
		GsmLogger& Logger() 
		{
			return _logger;
		}
		bool IsAsync;
		SimcomAtCommands(Stream& serial, UpdateBaudRateCallback updateBaudRateCallback);

		// Serial methods
		bool EnsureModemConnected(long requestedBaudRate);
		int FindCurrentBaudRate();
		void OnDataReceived(DataReceivedCallback onDataReceived);
		bool GarbageOnSerialDetected();

		// Standard modem functions
		AtResultType SetBaudRate(uint32_t baud);

		AtResultType At();
		AtResultType GenericAt(int timeout, const __FlashStringHelper* command,...);

		AtResultType Shutdown();
		AtResultType GetSimStatus(SimState &simStatus);
		AtResultType GetRegistrationStatus(GsmRegistrationState& registrationStatus);
		AtResultType GetOperatorName(FixedStringBase &operatorName, bool returnImsi = false);
		AtResultType FlightModeOn();
		AtResultType FlightModeOff();
		AtResultType SetRegistrationMode(RegistrationMode mode, const char * operatorName);
		AtResultType GetImei(FixedString20 &imei);
		AtResultType GetBatteryStatus(BatteryStatus &batteryStatus);
		AtResultType GetSignalQuality(int16_t &signalQuality);
		AtResultType SetEcho(bool echoEnabled);
		AtResultType SendSms(char *number, char *message);
		AtResultType Call(char *number);
		AtResultType GetIncomingCall(IncomingCallInfo &callInfo);
		
		// USSD
		AtResultType SendUssdWaitResponse(char *ussd, FixedString150& response);
		// TCP/UDP
		AtResultType GetIpState(SimcomIpState &ipState);
		AtResultType GetIpAddress(GsmIp &ipAddress);
		AtResultType GetRxMode(bool & isRxManual);
		AtResultType SetRxMode(bool isRxManual);
		AtResultType GetCipmux(bool &cipmux);
		AtResultType SetCipmux(bool cipmux);
		AtResultType GetCipQuickSend(bool &cipqsend);
		AtResultType SetSipQuickSend(bool cipqsend);
		AtResultType SetTransparentMode(bool transparentMode);
		AtResultType BeginConnect(ProtocolType protocol, uint8_t mux, const char *address, int port);
		AtResultType Read(int mux, FixedStringBase& outputBuffer);
		AtResultType Send(int mux, FixedStringBase& data, uint16_t &sentBytes);
		AtResultType CloseConnection(uint8_t mux);
		AtResultType GetConnectionInfo(uint8_t mux, ConnectionInfo &connectionInfo);

		// GPRS
		AtResultType SetApn(const char *apnName, const char *username, const char *password);
		AtResultType AttachGprs();
		AtResultType Cipshut();

		void wait(uint64_t millis);
};


#endif
