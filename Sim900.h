#ifndef __SIM_900_H__
#define __SIM_900_H__

#include "ParserSim900.h"
#include <Stream.h>
#include <Arduino.h>
#include <stdio.h>
#include <stdarg.h>
#include "Functions/FunctionBase.h"
#include "Sim900Context.h"
#include "SequenceDetector.h"

class Sim900: public Sim900Context
{
	private:
		int _powerPin;
		int _statusPin;
		
		// buffer for incoming data, used because fucking +++ needs 1000ms wait before issuing
		char _dataBuffer[DATA_BUFFER_SIZE];
public:
		int dataBufferHead;
		int dataBufferTail;
		
		int UnwriteDataBuffer();
		void WriteDataBuffer(char c);
		int ReadDataBuffer();
		FILE dataStream;
		Stream &ds;
	
		Sim900(Stream* serial, int powerPin, Stream& debugStream);
		
		static Stream* ser;
		void data_printf(const __FlashStringHelper *str, ...);
		ParserSim900 parser;
		
		void PrintEscapedChar(char c);
		void PrintDataByte(uint8_t data);
		// Standard modem functions
		int SetBaudRate(uint32_t baud);
		int TurnOn();
		void Shutdown();
		int GetRegistrationStatus();
		int GetOperatorName();
		int GetIMEI();
		int getSignalQuality();
		int ExecuteCommand_P(const __FlashStringHelper* command);		
		int SetEcho(bool echoEnabled);
		int SendSms(char *number, char *message);
		int Call(char *number);
		int PopCommandResult(int timeout);
		
		// Tcpip functions
		int GetIpStatus();
		int SetTransparentMode(bool transparentMode);
		int SetApn(const char *apnName, const char *username, const char *password);		
		int AttachGprs();
		int GetIpAddress();
		int StartTransparentIpConnection(const char *address, int port);
		int CloseConnection();
		int Cipshut();
		
		int ExecuteFunction(FunctionBase &function);
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
		int SwitchToCommandMode();
		int SwitchToCommandModeDropData();
		int SwitchToDataMode();
		void wait(int millis);
};


#endif
