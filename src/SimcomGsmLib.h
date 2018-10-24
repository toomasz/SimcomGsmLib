#ifndef _SIMCOM_GSM_LIB_H
#define _SIMCOM_GSM_LIB_H

#include <Stream.h>
#include <Arduino.h>
#include <stdio.h>
#include <stdarg.h>
#include "Functions/FunctionBase.h"
#include "Parsing/SequenceDetector.h"
#include "Parsing/SimcomResponseParser.h"
#include "Parsing/ParserContext.h"
#include "S900Socket.h"
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

		void SendAt_P(AtCommand commandType, const __FlashStringHelper *command, ...);
		AtResultType PopCommandResult(int timeout);
		AtResultType PopCommandResult();
public:
		SimcomGsm(Stream& serial, UpdateBaudRateCallback updateBaudRateCallback);
		bool EnsureModemConnected(long baudRate);

		int FindCurrentBaudRate();		

		void SetLogCallback(GsmLogCallback onLog)
		{
			_logger.SetLogCallback(onLog);
		}

		//void data_printf(const __FlashStringHelper *str, ...);
		void PrintDataByte(uint8_t data);

		// Standard modem functions
		AtResultType SetBaudRate(uint32_t baud);

		AtResultType At();
		AtResultType GenericAt(const __FlashStringHelper* command,...);
		AtResultType Shutdown();
		AtResultType GetRegistrationStatus(GsmRegistrationState& networkStatus);
		AtResultType GetOperatorName(FixedStringBase &operatorName, bool returnImsi = false);
		AtResultType SetRegistrationMode(RegistrationMode mode, const char * operatorName);
		AtResultType GetImei(FixedString20 &imei);
		AtResultType GetBatteryStatus(BatteryStatus &batteryStatus);
		AtResultType GetSignalQuality(int16_t &signalQuality);
		AtResultType SetEcho(bool echoEnabled);
		AtResultType SendSms(char *number, char *message);
		AtResultType Call(char *number);
		AtResultType GetIncomingCall(IncomingCallInfo &callInfo);
		

		AtResultType SendUssdWaitResponse(char *ussd, FixedString150& response);
		// Tcpip functions
		AtResultType GetIpState(SimcomIpState &ipStatus);
		AtResultType GetIpAddress(GsmIp &ipAddress);
		AtResultType GetCipmux(bool &cipmux);
		AtResultType SetTransparentMode(bool transparentMode);
		AtResultType SetApn(const char *apnName, const char *username, const char *password);
		AtResultType SetCipmux(bool cipmux);
		AtResultType AttachGprs();

		AtResultType BeginConnect(ProtocolType protocol, uint8_t mux, const char *address, int port);		
		AtResultType CloseConnection(uint8_t mux);
		AtResultType GetConnectionInfo(uint8_t mux, ConnectionInfo &connectionInfo);

		AtResultType Cipshut();

		// Data/command mode switching
		AtResultType SwitchToCommandMode();
		AtResultType SwitchToCommandModeDropData();
		AtResultType SwitchToDataMode();

		
		AtResultType ExecuteFunction(FunctionBase &function);

		unsigned long lastDataWrite;	
		void wait(int millis);
};


#endif
