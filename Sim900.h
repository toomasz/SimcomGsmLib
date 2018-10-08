#ifndef __SIM_900_H__
#define __SIM_900_H__

#include <Stream.h>
#include <Arduino.h>
#include <stdio.h>
#include <stdarg.h>
#include "Functions/FunctionBase.h"
#include "Sim900Context.h"
#include "SequenceDetector.h"
#include "S900Socket.h"
#include "ParserSim900.h"

#include <pgmspace.h>

class S900Socket;
class ParserSim900;

class Sim900: public Sim900Context
{
private:
		int _powerPin;
		int _statusPin;
		Stream &serial;
		// buffer for incoming data, used because fucking +++ needs 1000ms wait before issuing
		char _dataBuffer[DATA_BUFFER_SIZE];
		int _currentBaudRate;
		void Log_P(const __FlashStringHelper *command, ...);

		void SendAt_P(int commandType, const __FlashStringHelper *command, ...);
		UpdateBaudRateCallback _updateBaudRateCallback;
		GsmLogCallback _onLog;
public:
		Sim900(Stream& serial, UpdateBaudRateCallback updateBaudRateCallback);
		

		int dataBufferHead;
		int dataBufferTail;

		int FindCurrentBaudRate();
		
		int UnwriteDataBuffer();
		void WriteDataBuffer(char c);
		int ReadDataBuffer();
		FILE dataStream;

		void SetLogCallback(GsmLogCallback onLog)
		{
			_onLog;
		}

		//void data_printf(const __FlashStringHelper *str, ...);
		ParserSim900& parser;
		void PrintDataByte(uint8_t data);

		// Standard modem functions
		AtResultType SetBaudRate(uint32_t baud);
		bool EnsureModemConnected();

		AtResultType At();
		AtResultType Shutdown();
		AtResultType GetRegistrationStatus(GsmNetworkStatus& networkStatus);
		AtResultType GetOperatorName();
		AtResultType GetIMEI();
		AtResultType GetBatteryStatus();
		AtResultType getSignalQuality();
		AtResultType SetEcho(bool echoEnabled);
		AtResultType SendSms(char *number, char *message);
		AtResultType Call(char *number);
		AtResultType EnableCallerId();
		AtResultType PopCommandResult(int timeout);
		AtResultType SendUssdWaitResponse(char *ussd, char*response, int responseBufferLength);
		
		// Tcpip functions
		AtResultType GetIpStatus();
		AtResultType SetTransparentMode(bool transparentMode);
		AtResultType SetApn(const char *apnName, const char *username, const char *password);
		AtResultType AttachGprs();
		AtResultType GetIpAddress();
		AtResultType StartTransparentIpConnection(const char *address, int port, S900Socket *socket);
		
		AtResultType CloseConnection();
		AtResultType Cipshut();

		// Data/command mode switching
		AtResultType SwitchToCommandMode();
		AtResultType SwitchToCommandModeDropData();
		AtResultType SwitchToDataMode();
		
		AtResultType ExecuteFunction(FunctionBase &function);
		// Transparent mode data functions
		bool DataAvailable();
		int DataRead();
		unsigned long lastDataWrite;
	
		void DataWrite(const __FlashStringHelper* data);
		void DataWrite(char* data);	
		void DataWrite(char *data, int length);
	
		void DataWriteNumber( int c );
		void DataWriteNumber( uint16_t c );
	    void DataWrite(char c);
		void DataEndl();	

		bool IsPoweredUp();
		bool commandBeforeRN;
		void wait(int millis);
};


#endif
