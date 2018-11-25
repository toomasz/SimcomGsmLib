#ifndef _SIMCOM_GSM_LIB_H
#define _SIMCOM_GSM_LIB_H

#include <Stream.h>
#include <Arduino.h>
#include <stdio.h>
#include <stdarg.h>
#include "Parsing/SequenceDetector.h"
#include "Parsing/SimcomResponseParser.h"
#include "Parsing/ParserContext.h"
#include "CircularDataBuffer.h"
#include "GsmLogger.h"
#include "SimcomGsmTypes.h"
#include <pgmspace.h>

class S900Socket;

class SimcomGsm
{
private:
		Stream &_serial;
		int _currentBaudRate;
		GsmLogger _logger;
		SimcomResponseParser _parser;
		UpdateBaudRateCallback _updateBaudRateCallback;
		ParserContext _parserContext;
		// buffer for incoming data, used because fucking +++ needs 1000ms wait before issuing
		CircularDataBuffer _dataBuffer;
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
		SimcomGsm(Stream& serial, UpdateBaudRateCallback updateBaudRateCallback);
		bool EnsureModemConnected(long baudRate);

		int FindCurrentBaudRate();
		void OnDataReceived(DataReceivedCallback onDataReceived);

		// Standard modem functions
		AtResultType SetBaudRate(uint32_t baud);

		AtResultType At();
		AtResultType GenericAt(int timeout, const __FlashStringHelper* command,...);

		AtResultType Shutdown();
		AtResultType GetSimStatus(SimState &simStatus);
		AtResultType GetRegistrationStatus(GsmRegistrationState& networkStatus);
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
		

		AtResultType SendUssdWaitResponse(char *ussd, FixedString150& response);
		// TCP/UDP
		AtResultType GetIpState(SimcomIpState &ipStatus);
		AtResultType GetIpAddress(GsmIp &ipAddress);
		AtResultType GetRxMode(bool & isRxManual);
		AtResultType SetRxMode(bool isRxManual);
		AtResultType GetCipmux(bool &cipmux);
		AtResultType SetCipmux(bool cipmux);
		AtResultType GetCipQuickSend(bool &cipqsend);
		AtResultType SetSipQuickSend(bool cipqsend);

		AtResultType SetTransparentMode(bool transparentMode);
		AtResultType SetApn(const char *apnName, const char *username, const char *password);
		AtResultType AttachGprs();

		AtResultType BeginConnect(ProtocolType protocol, uint8_t mux, const char *address, int port);		
		AtResultType Read(int mux, FixedStringBase& outputBuffer);
		AtResultType CloseConnection(uint8_t mux);
		AtResultType GetConnectionInfo(uint8_t mux, ConnectionInfo &connectionInfo);
		AtResultType Cipshut();

		unsigned long lastDataWrite;	
		void wait(int millis);
		bool GarbageOnSerialDetected();
};


#endif
