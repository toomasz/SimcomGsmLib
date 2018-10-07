/*
 * FunctionDebug.h
 *
 * Created: 2014-02-12 16:44:03
 *  Author: Tomek
 */ 
#include <Arduino.h>
#include "ParserSim900.h"
#include "Sim900Strings.h"
#ifndef FUNCTIONDEBUG_H_
#define FUNCTIONDEBUG_H_

class FunctionDebug
{
	Stream& debugStream;
	public:
	
	FunctionDebug(Stream &stream):debugStream(stream)
	{
		
	}
	void DebugPrefix(int function)
	{
		if(function == AT_CIPSTATUS)
			debugStream.print(F("Cipstatus():"));
		if(function == AT_CREG)
			debugStream.print("GetRegistrationStatus():");
		if(function == AT_CIFSR)
		    debugStream.print(F("GetIpAddress():"));
	}
	void DebugResult(int function, int result)
	{
		/*if(function == AT_CIPSTATUS)
		{
			debugStream.println(Sim900Strings::IpStatus2Pstr(result));
		}
		if(function == AT_CREG)
		{
			debugStream.println(Sim900Strings::RegStatus2Pstr(result));
		}*/
	}
	
	void DebugAtResult(AtResultType result)
	{
		if(result == AtResultType::Success)
			debugStream.println(F("S900_TIMEOUT"));
		if(result == AtResultType::Error)
			debugStream.println(F("S900_ERR"));
		if(result == AtResultType::Timeout)
			debugStream.println(F("S900_OK"));
	}
	
};



#endif /* FUNCTIONDEBUG_H_ */