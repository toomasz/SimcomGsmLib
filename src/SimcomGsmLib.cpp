#include "SimcomGsmLib.h"

SimcomGsm::SimcomGsm(Stream& serial, UpdateBaudRateCallback updateBaudRateCallback) :
_serial(serial),
_parser(_dataBuffer, _parserContext, _logger)
{
	lastDataWrite = 0;
	_updateBaudRateCallback = updateBaudRateCallback;
	_currentBaudRate = 0;
}

AtResultType SimcomGsm::GetRegistrationStatus(GsmNetworkStatus& networkStatus)
{
	SendAt_P(AtCommand::Creg ,F("AT+CREG?"));

	auto result = PopCommandResult();
	if (result == AtResultType::Success)
	{
		networkStatus = _parser._lastGsmResult;
	}
	return result;
}
AtResultType SimcomGsm::At(const __FlashStringHelper* command)
{
	SendAt_P(AtCommand::Generic, command);
	auto result = PopCommandResult();
	return result;
}
AtResultType SimcomGsm::GetOperatorName(FixedStringBase &operatorName, bool returnImsi)
{
	SendAt_P(AtCommand::Cops, F("AT+COPS?"));
	_parserContext.OperatorName = &operatorName;

	auto result = PopCommandResult();
	if (result != AtResultType::Success)
	{
		return result;
	}

	if (_parserContext.IsOperatorNameReturnedInImsiFormat == returnImsi)
	{
		return result;
	}

	if (returnImsi)
	{
		At(F("AT+COPS=0,2"));
	}
	else
	{
		At(F("AT+COPS=0,0"));
	}

	SendAt_P(AtCommand::Cops, F("AT+COPS?"));
	result = PopCommandResult();


	return result;
}

AtResultType SimcomGsm::GetSignalQuality(int16_t& signalQuality)
{
	_parserContext.CsqSignalQuality = &signalQuality;
	SendAt_P(AtCommand::Csq, F("AT+CSQ"));
	auto result = PopCommandResult();
	return result;
}

AtResultType SimcomGsm::GetBatteryStatus(BatteryStatus &batteryStatus)
{
	_parserContext.BatteryInfo = &batteryStatus;
	SendAt_P(AtCommand::Cbc,F("AT+CBC"));
	return PopCommandResult();
}

AtResultType SimcomGsm::GetIpState(SimcomIpState &ipState)
{
	_parserContext.IpState = &ipState;
	SendAt_P(AtCommand::Cipstatus, F("AT+CIPSTATUS"));
	return PopCommandResult();	
}

AtResultType SimcomGsm::GetIpAddress(FixedString20& ipAddress)
{
	_parserContext.IpAddress = &ipAddress;
	SendAt_P(AtCommand::Cifsr, F("AT+CIFSR"));
	return PopCommandResult();
}

AtResultType SimcomGsm::GetCipmux(bool& cipmux)
{
	SendAt_P(AtCommand::Cipmux, F("AT+CIPMUX?"));
	return PopCommandResult();
}

AtResultType SimcomGsm::AttachGprs()
{
	SendAt_P(AtCommand::Generic, F("AT+CIICR"));
	return PopCommandResult(60000);
}

AtResultType SimcomGsm::StartTransparentIpConnection(const char *address, int port, S900Socket *socket = 0 )
{
	_dataBuffer.Clear();

	// Execute command like AT+CIPSTART="TCP","example.com","80"
	SendAt_P(AtCommand::Cipstart, F("AT+CIPSTART=\"TCP\",\"%s\",\"%d\""), address, port);

	if (socket != 0)
	{
		socket->s900 = this;
	}
	return PopCommandResult(60000);
}

AtResultType SimcomGsm::CloseConnection()
{
	SendAt_P(AtCommand::Cipclose, F("AT+CIPCLOSE=1"));
	return PopCommandResult();
}


/* returns true if any data is available to read from transparent connection */
bool SimcomGsm::DataAvailable()
{
	if (_dataBuffer.DataAvailable())
	{
		return true;
	}
	if (_serial.available())
	{
		return true;
	}
	return false;
}
void SimcomGsm::PrintDataByte(uint8_t data) // prints 8-bit data in hex
{
	char tmp[3];
	byte first;
	byte second;

	first = (data >> 4) & 0x0f;
	second = data & 0x0f;

	tmp[0] = first+48;
	tmp[1] = second+48;
	if (first > 9) tmp[0] += 39;
	if (second > 9) tmp[1] += 39;
	
	tmp[2] = 0;
//	ds.write(' ');
//	ds.print(tmp);
//	ds.write(' ');
}

int SimcomGsm::DataRead()
{
	int ret = _dataBuffer.ReadDataBuffer();
	if(ret != -1)
	{
		//PrintDataByte(ret);
		return ret;
	}
	ret = _serial.read();
	if(ret != -1)
	{
	//	PrintDataByte(ret);
	}
	return ret;
}

AtResultType SimcomGsm::SwitchToCommandMode()
{
	_parser.SetCommandType(AtCommand::SwitchToCommand);
	_dataBuffer.commandBeforeRN = true;
	// +++ escape sequence needs to wait 1000ms after last data was sent via transparent connection
	// in the meantime data from tcp connection may arrive so we read it here to dataBuffer
	// ex: lastDataWrite = 1500
	// loop will exit when millis()-1500<1000 so when millis is 2500
	
	delay(1000);
  /* while(ser->available() || (millis()-lastDataWrite) < 1000)
	{
		while(ser->available())
		{
			int c = ser->read();
			pr("\nsc_data: %c\n", (char)c);
			WriteDataBuffer(c);
		}
	}*/
	
	_serial.print(F("+++"));
	lastDataWrite = millis();
	return PopCommandResult(500);
}

AtResultType SimcomGsm::SwitchToCommandModeDropData()
{
	_parser.SetCommandType(AtCommand::SwitchToCommand);
	_serial.flush();
	while (_serial.available())
	{
		_serial.read();
	}
	delay(1500);
	while (_serial.available())
	{
		_serial.read();
	}

	_serial.print(F("+++"));

	return PopCommandResult(500);
}

/*
Switches to data mode ATO
Return values S900_OK, S900_ERROR, S900_TIMEOUT
*/
AtResultType SimcomGsm::SwitchToDataMode()
{
	SendAt_P(AtCommand::SwitchToData, F("ATO"));

	auto result = PopCommandResult();
	if(result == AtResultType::Success)
	{
		delay(100);
		while(_serial.available())
		{
			int c = _serial.read();
			//pr("\nsd_data: %c\n", (char)c);
			//ds.print("s_data: "); ds.println((int)c);
			_dataBuffer.WriteDataBuffer(c);
		}
		
	}
	lastDataWrite = millis();
	return result;
}
AtResultType SimcomGsm::PopCommandResult()
{
	return PopCommandResult(AT_DEFAULT_TIMEOUT);
}
AtResultType SimcomGsm::PopCommandResult( int timeout )
{
	unsigned long start = millis();
	while(_parser.commandReady == false && (millis()-start) < (unsigned long)timeout)
	{
		if(_serial.available())
		{
			char c = _serial.read();
			_parser.FeedChar(c);
		}
	}

	auto commandResult = _parser.GetAtResultType();
	_parser.SetCommandType(0);
	auto elapsedMs = millis() - start;
	_logger.Log_P(F(" --- %d ms ---"), elapsedMs);
	return commandResult;
}
/*
Disables/enables echo on serial port
*/
AtResultType SimcomGsm::SetEcho(bool echoEnabled)
{
	if (echoEnabled)
	{
		SendAt_P(AtCommand::Generic, F("ATE1"));
	}
	else
	{
		SendAt_P(AtCommand::Generic, F("ATE0"));
	}

	auto r = PopCommandResult();
	delay(100); // without 100ms wait, next command failed, idk wky
	return r;
}
/*
Set gsm modem to use transparent mode
*/
AtResultType SimcomGsm::SetTransparentMode( bool transparentMode )
{	
	if (transparentMode)
	{
		SendAt_P(AtCommand::Generic, F("AT+CIPMODE=1"));
	}
	else
	{
		SendAt_P(AtCommand::Generic, F("AT+CIPMODE=0"));
	}
		
	return PopCommandResult();
}

AtResultType SimcomGsm::SetApn(const char *apnName, const char *username,const char *password )
{
	SendAt_P(AtCommand::Generic, F("AT+CSTT=\"%s\",\"%s\",\"%s\""), apnName, username, password);
	return PopCommandResult();
}

void SimcomGsm::DataWrite( const __FlashStringHelper* data )
{
	_serial.print(data);
	lastDataWrite = millis();
}

void SimcomGsm::DataWrite( char* data )
{
	_serial.print(data);
	lastDataWrite = millis();
}

void SimcomGsm::DataWrite( char *data, int length )
{
	_serial.write((unsigned char*)data, length);
	lastDataWrite = millis();
}

void SimcomGsm::DataWrite( char c )
{
	_serial.write(c);
	lastDataWrite = millis();
}


void SimcomGsm::DataEndl()
{
	_serial.print(F("\r\n"));
	_serial.flush();
	lastDataWrite = millis();
}
AtResultType SimcomGsm::At()
{
	SendAt_P(AtCommand::Generic, F("AT"));
	return PopCommandResult(30);
}

AtResultType SimcomGsm::SetBaudRate(uint32_t baud)
{
	SendAt_P(AtCommand::Generic, F("AT+IPR=%d"), baud);
	return PopCommandResult();
}
bool SimcomGsm::EnsureModemConnected(long requestedBaudRate)
{
	auto atResult = At();

	if (_currentBaudRate == 0 || atResult != AtResultType::Success)
	{
		_currentBaudRate = FindCurrentBaudRate();
		if (_currentBaudRate != 0)
		{
			_logger.Log_P(F("Found baud rate = %d"), _currentBaudRate);
			if (SetBaudRate(requestedBaudRate) == AtResultType::Success)
			{
				_currentBaudRate = requestedBaudRate;
				_logger.Log_P(F("set baud rate to = %d"), _currentBaudRate);
				At();
				SetEcho(false);
				return true;
			}
			_logger.Log_P(F("Failed to update baud rate = %d"), _currentBaudRate);
			return true;
		}
	}
	return atResult == AtResultType::Success;
}
AtResultType SimcomGsm::GetImei(FixedString20 &imei)
{
	_parserContext.Imei = &imei;
	SendAt_P(AtCommand::Gsn, F("AT+GSN"));
	return PopCommandResult();
}

void SimcomGsm::wait(int ms)
{
	unsigned long start = millis();
	while ((millis() - start) <= (unsigned long)ms)
	{
		if (_serial.available())
		{
			_parser.FeedChar(_serial.read());
		}
	}
}

AtResultType SimcomGsm::ExecuteFunction(FunctionBase &function)
{
	_parser.SetCommandType(&function);
	_serial.println(function.getCommand());
	
	auto initialResult = PopCommandResult(function.functionTimeout);
	
	if (initialResult == AtResultType::Success)
	{
		return AtResultType::Success;
	}
	if(function.GetInitSequence() == NULL)
		return initialResult;
		
	char *p= (char*)function.GetInitSequence();
	while(pgm_read_byte(p) != 0)
	{
			
	//	ds.print(F("Exec: "));
			
		//printf("Exec: %s\n", p);
		wait(100);
		SendAt_P(AtCommand::Generic, (__FlashStringHelper*)p);
		auto r = PopCommandResult();
		if(r != AtResultType::Success)
		{
				
			return r;
		}

		while(pgm_read_byte(p) != 0)			
			p++;
			
		p++;
	}
	delay(500);
	_parser.SetCommandType(&function);
	_serial.println(function.getCommand());
	return PopCommandResult(function.functionTimeout);
}

AtResultType SimcomGsm::SendSms(char *number, char *message)
{
	SendAt_P(AtCommand::Generic, F("AT+CMGS=\"%s\""), number);
	
	uint64_t start = millis();
	// wait for >
	while (_serial.read() != '>')
		if (millis() - start > 200)
			return AtResultType::Error;
	_serial.print(message);
	_serial.print('\x1a');
	return PopCommandResult();
}
AtResultType SimcomGsm::SendUssdWaitResponse(char *ussd, FixedString150& response)
{
	_parserContext.UssdResponse = &response;
	SendAt_P(AtCommand::Cusd, F("AT+CUSD=1,\"%s\""), ussd);
	auto result = PopCommandResult(10000);
}
void SimcomGsm::SendAt_P(AtCommand commnd, const __FlashStringHelper* command, ...)
{
	_parser.SetCommandType(commnd);

	va_list argptr;
	va_start(argptr, command);

	char commandBuffer[200];
	vsnprintf_P(commandBuffer, 200, (PGM_P)command, argptr);
	_logger.Log_P(F(" => %s"), commandBuffer);
	_serial.println(commandBuffer);

	va_end(argptr);
}
int SimcomGsm::FindCurrentBaudRate()
{
	if (_updateBaudRateCallback == nullptr)
	{
		return 0;
	}
	int i = 0;
	int baudRate = 0;
	do
	{
		baudRate = _defaultBaudRates[i];
		_logger.Log_P(F("Trying baud rate: %d"), baudRate);
		yield();
		_updateBaudRateCallback(baudRate);
		yield();
		if (At() == AtResultType::Success)
		{
			_logger.Log_P(F(" Found baud rate: %d"), baudRate);
			return baudRate;
		}
		i++;
	} 
	while (_defaultBaudRates[i] != 0);

	return 0;
}

AtResultType SimcomGsm::Cipshut()
{
	SendAt_P(AtCommand::Cipshut, F("AT+CIPSHUT"));
	return PopCommandResult();
}

void SimcomGsm::DataWriteNumber(int c)
{
	_serial.print(c);
	lastDataWrite = millis();
}
void SimcomGsm::DataWriteNumber(uint16_t c)
{
	_serial.print(c);
	lastDataWrite = millis();
}

AtResultType SimcomGsm::Call(char *number)
{
	SendAt_P(AtCommand::Generic, F("ATD%s;"), number);
	return PopCommandResult();
}

AtResultType SimcomGsm::GetIncomingCall(IncomingCallInfo & callInfo)
{
	_parserContext.CallInfo = &callInfo;
	SendAt_P(AtCommand::Clcc, F("AT+CLCC"));
	auto result = PopCommandResult();
	return result;
}

AtResultType SimcomGsm::Shutdown()
{
	SendAt_P(AtCommand::Generic, F("AT+CPOWD=0"));
	return PopCommandResult();
}







































