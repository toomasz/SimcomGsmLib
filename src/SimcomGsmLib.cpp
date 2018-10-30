#include "SimcomGsmLib.h"
#include "MappingHelpers.h"

SimcomGsm::SimcomGsm(Stream& serial, UpdateBaudRateCallback updateBaudRateCallback) :
_serial(serial),
_parser(_dataBuffer, _parserContext, _logger)
{
	lastDataWrite = 0;
	_updateBaudRateCallback = updateBaudRateCallback;
	_currentBaudRate = 0;
}
AtResultType SimcomGsm::GetSimStatus(SimState &simStatus)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	SendAt_P(AtCommand::Cpin, F("AT+CPIN?"));
	auto result = PopCommandResult();
	if (result == AtResultType::Success)
	{
		simStatus = _parserContext.SimStatus;
	}
	return result;
}

AtResultType SimcomGsm::GetRegistrationStatus(GsmRegistrationState& registrationStatus)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	SendAt_P(AtCommand::Creg ,F("AT+CREG?"));

	auto result = PopCommandResult();
	if (result == AtResultType::Success)
	{
		registrationStatus = _parserContext.RegistrationStatus;
	}
	return result;
}
AtResultType SimcomGsm::GenericAt(const __FlashStringHelper* command, ...)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	_parser.SetCommandType(AtCommand::Generic);
	va_list argptr;
	va_start(argptr, command);

	char commandBuffer[200];
	vsnprintf_P(commandBuffer, 200, (PGM_P)command, argptr);	
	_logger.LogAt(F(" => %s"), commandBuffer);
	_serial.println(commandBuffer);
	auto result = PopCommandResult();

	va_end(argptr);	
	return result;
}
void SimcomGsm::SendAt_P(AtCommand commnd, const __FlashStringHelper* command, ...)
{
	_parser.SetCommandType(commnd);

	va_list argptr;
	va_start(argptr, command);

	char commandBuffer[200];
	vsnprintf_P(commandBuffer, 200, (PGM_P)command, argptr);
	_logger.LogAt(F(" => %s"), commandBuffer);
	_serial.println(commandBuffer);

	va_end(argptr);
}
AtResultType SimcomGsm::GetOperatorName(FixedStringBase &operatorName, bool returnImsi)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
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

	auto operatorFormat = returnImsi ? 2 : 0;	
	GenericAt(F("AT+COPS=3,%d"), operatorFormat);	
	SendAt_P(AtCommand::Cops, F("AT+COPS?"));
	return PopCommandResult();
}

AtResultType SimcomGsm::SetRegistrationMode(RegistrationMode mode, const char *operatorName)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	auto operatorFormat = _parserContext.IsOperatorNameReturnedInImsiFormat ? 2 : 0;
	// don't need to use AtCommand::Cops here, AT+COPS write variant returns OK/ERROR
	SendAt_P(AtCommand::Generic, F("AT+COPS=%d,%d,\"%s\""), mode, operatorFormat, operatorName);
	auto result = PopCommandResult(120000);
	return result;
}

AtResultType SimcomGsm::GetSignalQuality(int16_t& signalQuality)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	_parserContext.CsqSignalQuality = &signalQuality;
	SendAt_P(AtCommand::Csq, F("AT+CSQ"));
	auto result = PopCommandResult();
	return result;
}

AtResultType SimcomGsm::GetBatteryStatus(BatteryStatus &batteryStatus)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	_parserContext.BatteryInfo = &batteryStatus;
	SendAt_P(AtCommand::Cbc,F("AT+CBC"));
	return PopCommandResult();
}

AtResultType SimcomGsm::GetIpState(SimcomIpState &ipState)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	_parserContext.IpState = &ipState;
	SendAt_P(AtCommand::Cipstatus, F("AT+CIPSTATUS"));
	return PopCommandResult();	
}

AtResultType SimcomGsm::GetIpAddress(GsmIp& ipAddress)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	_parserContext.IpAddress = &ipAddress;
	SendAt_P(AtCommand::Cifsr, F("AT+CIFSR;E0"));
	return PopCommandResult();
}

AtResultType SimcomGsm::GetRxMode(bool& isRxManual)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	SendAt_P(AtCommand::CipRxGet, F("AT+CIPRXGET?"));
	auto result = PopCommandResult();
	if (result == AtResultType::Success)
	{
		isRxManual = _parserContext.IsRxManual;
	}
	return result;
}

AtResultType SimcomGsm::SetRxMode(bool isRxManual)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	SendAt_P(AtCommand::Generic, F("AT+CIPRXGET=%d"), isRxManual ? 1 : 0);

	return PopCommandResult();
}

AtResultType SimcomGsm::GetCipmux(bool& cipmux)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	SendAt_P(AtCommand::Cipmux, F("AT+CIPMUX?"));
	auto result = PopCommandResult();
	if (result == AtResultType::Success)
	{
		cipmux = _parserContext.Cipmux;
	}
	return result;
}

AtResultType SimcomGsm::SetCipmux(bool cipmux)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	SendAt_P(AtCommand::Generic, F("AT+CIPMUX=%d"), cipmux ? 1 : 0);
	
	return PopCommandResult();	
}
AtResultType SimcomGsm::AttachGprs()
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	SendAt_P(AtCommand::Generic, F("AT+CIICR"));
	return PopCommandResult(60000);
}

AtResultType SimcomGsm::PopCommandResult()
{
	return PopCommandResult(AT_DEFAULT_TIMEOUT);
}
AtResultType SimcomGsm::PopCommandResult(int timeout)
{
	unsigned long start = millis();
	while(_parser.commandReady == false && (millis()-start) < (unsigned long)timeout)
	{
		if(_serial.available())
		{
			char c = _serial.read();
			//Serial.printf("c: %c\n", c);
			_parser.FeedChar(c);
		}
	}

	auto commandResult = _parser.GetAtResultType();
	auto elapsedMs = millis() - start;	
	_logger.LogAt(F(" --- %d ms ---"), elapsedMs);
	if (commandResult == AtResultType::Timeout)
	{
		_logger.Log(F("                      --- !!!TIMEOUT!!! ---      "), elapsedMs);
	}
	if (commandResult == AtResultType::Error)
	{
		_logger.Log(F("                      --- !!!ERROR!!! ---      "), elapsedMs);
	}
	return commandResult;
}
/*
Disables/enables echo on serial port
*/
AtResultType SimcomGsm::SetEcho(bool echoEnabled)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
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

AtResultType SimcomGsm::SetTransparentMode(bool transparentMode)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	SendAt_P(AtCommand::Generic, F("AT+CIPMODE=%d"), transparentMode ? 1:0);
	return PopCommandResult();
}

AtResultType SimcomGsm::SetApn(const char *apnName, const char *username,const char *password )
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	SendAt_P(AtCommand::Generic, F("AT+CSTT=\"%s\",\"%s\",\"%s\""), apnName, username, password);
	return PopCommandResult();
}

AtResultType SimcomGsm::At()
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	SendAt_P(AtCommand::Generic, F("AT"));
	return PopCommandResult(30);
}

void SimcomGsm::OnDataReceived(DataReceivedCallback onDataReceived)
{
	_parser.OnDataReceived(onDataReceived);
}

AtResultType SimcomGsm::SetBaudRate(uint32_t baud)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	SendAt_P(AtCommand::Generic, F("AT+IPR=%d"), baud);
	return PopCommandResult();
}
bool SimcomGsm::EnsureModemConnected(long requestedBaudRate)
{
	if (_parser.IsReceivingData())
	{
		return true;
	}
	auto atResult = At();

	int n = 8;
	while (atResult != AtResultType::Success && n--> 0)
	{
		delay(50);
		atResult = At();		
	}

	if (_currentBaudRate == 0 || atResult == AtResultType::Timeout)
	{
		_currentBaudRate = FindCurrentBaudRate();
		if (_currentBaudRate != 0)
		{
			_logger.Log(F("Found baud rate = %d"), _currentBaudRate);
			if (SetBaudRate(requestedBaudRate) == AtResultType::Success)
			{
				_currentBaudRate = requestedBaudRate;
				_logger.Log(F("set baud rate to = %d"), _currentBaudRate);
				At();
				SetEcho(false);
				return true;
			}
			_logger.Log(F("Failed to update baud rate = %d"), _currentBaudRate);
			return true;
		}
	}
	return atResult == AtResultType::Success;
}
AtResultType SimcomGsm::GetImei(FixedString20 &imei)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
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
			auto c = _serial.read();
			//Serial.printf("c: %c\n", c);
			_parser.FeedChar(c);
		}
	}
}

AtResultType SimcomGsm::ExecuteFunction(FunctionBase &function)
{
//	_parser.SetCommandType(&function);
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
//	_parser.SetCommandType(&function);
	_serial.println(function.getCommand());
	return PopCommandResult(function.functionTimeout);
}

AtResultType SimcomGsm::SendSms(char *number, char *message)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
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
		yield();
		baudRate = _defaultBaudRates[i];
		_logger.Log(F("Trying baud rate: %d"), baudRate);
		_updateBaudRateCallback(baudRate);
		if (At() == AtResultType::Success)
		{
			_logger.Log(F(" Found baud rate: %d"), baudRate);
			return baudRate;
		}
		i++;
	} 
	while (_defaultBaudRates[i] != 0);

	return 0;
}

AtResultType SimcomGsm::Cipshut()
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	SendAt_P(AtCommand::Cipshut, F("AT+CIPSHUT"));
	return PopCommandResult();
}

AtResultType SimcomGsm::Call(char *number)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
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
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	SendAt_P(AtCommand::Generic, F("AT+CPOWD=0"));
	return PopCommandResult();
}


AtResultType SimcomGsm::BeginConnect(ProtocolType protocol, uint8_t mux, const char *address, int port)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	SendAt_P(AtCommand::Generic, 
		F("AT+CIPSTART=%d,\"%s\",\"%s\",\"%d\""),
		mux, ProtocolToStr(protocol), address, port);	
	return PopCommandResult(60000);
}

AtResultType SimcomGsm::Read(int mux, FixedStringBase& outputBuffer)
{
	_parserContext.CipRxGetBuffer = &outputBuffer;
	SendAt_P(AtCommand::CipRxGetRead,F("AT+CIPRXGET=2,%d,%d"), mux, outputBuffer.capacity());
	return PopCommandResult();
}

AtResultType SimcomGsm::CloseConnection(uint8_t mux)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	SendAt_P(AtCommand::Cipclose, F("AT+CIPCLOSE=%d"), mux);
	return PopCommandResult();
}

AtResultType SimcomGsm::GetConnectionInfo(uint8_t mux, ConnectionInfo &connectionInfo)
{
	if (_parser.IsReceivingData())
	{
		return AtResultType::Error;
	}
	_parserContext.CurrentConnectionInfo = &connectionInfo;
	SendAt_P(AtCommand::CipstatusSingleConnection, F("AT+CIPSTATUS=%d"), mux);
	return PopCommandResult();
}







































