#ifndef _SIMCOM_AT_COMMANDS_H
#define _SIMCOM_AT_COMMANDS_H

#include <Stream.h>
#include <Arduino.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
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
		uint64_t _currentBaudRate;
		SimcomResponseParser _parser;
		UpdateBaudRateCallback _updateBaudRateCallback;
		SetDtrCallback _setDtrCallback;
		ParserContext _parserContext;
		FixedString50 _currentCommand;
		uint64_t _lastIncomingByteTime;
		void SendAt_P(AtCommand commandType, const __FlashStringHelper *command, ...);
		void SendAt_P(AtCommand commandType, bool expectEcho, const __FlashStringHelper *command, ...);

		AtResultType PopCommandResult(bool ensureDelay, uint64_t timeout);
		AtResultType PopCommandResult(bool ensureDelay = false);
		void ReadCharAndFeedParser();
		void ReadCharAndIgnore();
		bool _isInSleepMode;
protected:
		GsmLogger _logger;

public:
		GsmLogger& Logger() 
		{
			return _logger;
		}
		bool IsAsync;
		SimcomAtCommands(Stream& serial, UpdateBaudRateCallback updateBaudRateCallback, SetDtrCallback setDtrCallback = nullptr);

		// Serial methods
		bool EnsureModemConnected(uint64_t requestedBaudRate);
		uint64_t FindCurrentBaudRate();
		bool GarbageOnSerialDetected();

		// Standard modem functions
		AtResultType SetBaudRate(uint32_t baud);

		AtResultType At(uint32_t timeout = 60u, bool expectEcho = false);
		AtResultType GenericAt(uint64_t timeout, const __FlashStringHelper* command,...);
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
	
		// Calls
		AtResultType Call(char *number);
		AtResultType HangUp();
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
		AtResultType Read(int mux, FixedStringBase& outputBuffer, uint16_t& availableBytes);
		AtResultType Send(int mux, FixedStringBase& data, uint16_t index, uint16_t length, uint16_t &sentBytes);
		AtResultType Send(int mux, FixedStringBase& data, uint16_t &sentBytes);
		AtResultType CloseConnection(uint8_t mux);
		AtResultType GetConnectionInfo(uint8_t mux, ConnectionInfo &connectionInfo);

		void OnMuxEvent(void* ctx, MuxEventHandler muxEventHandler);
		void OnCipstatusInfo(void * ctx, MuxCipstatusInfoHandler muxCipstatusHandler);
		void OnGsmModuleEvent(void* ctx, OnGsmModuleEventHandler gsmModuleEventHandler);

		// Misc
		AtResultType GetTemperature(float& temperature);
		AtResultType EnableNetlight(bool enable);

		// Sleep mode
		bool IsInSleepMode();
		AtResultType EnterSleepMode();
		AtResultType ExitSleepMode();
		// GPRS
		AtResultType SetApn(const char *apnName, const char *username, const char *password);
		AtResultType AttachGprs();
		AtResultType Cipshut();

		bool SetDtr(bool value);
		void wait(uint64_t millis);
};


#endif
