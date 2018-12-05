#include "SimcomAtCommands.h"
#include "GsmLibHelpers.h"

SimcomAtCommands::SimcomAtCommands(Stream& serial, UpdateBaudRateCallback updateBaudRateCallback) :
_serial(serial),
_parser(_parserContext, _logger, serial),
IsAsync(false)
{
	_updateBaudRateCallback = updateBaudRateCallback;
	_currentBaudRate = 0;
}
AtResultType SimcomAtCommands::GetSimStatus(SimState &simStatus)
{
	SendAt_P(AtCommand::Cpin, F("AT+CPIN?"));
	const auto result = PopCommandResult();
	if (result == AtResultType::Success)
	{
		simStatus = _parserContext.SimStatus;
	}
	return result;
}

AtResultType SimcomAtCommands::GetRegistrationStatus(GsmRegistrationState& registrationStatus)
{	
	SendAt_P(AtCommand::Creg ,F("AT+CREG?"));

	const auto result = PopCommandResult();
	if (result == AtResultType::Success)
	{
		registrationStatus = _parserContext.RegistrationStatus;
	}
	return result;
}
AtResultType SimcomAtCommands::GenericAt(int timeout, const __FlashStringHelper* command, ...)
{	
	_parser.SetCommandType(AtCommand::Generic);
	va_list argptr;
	va_start(argptr, command);

	FixedString200 buffer;
	buffer.appendFormatV(command, argptr);
	_logger.LogAt(F(" => %s"), buffer.c_str());
	_currentCommand = buffer;
	_serial.println(buffer.c_str());

	const auto result = PopCommandResult(timeout);
	va_end(argptr);	
	return result;
}
void SimcomAtCommands::SendAt_P(AtCommand commandType, const __FlashStringHelper* command, ...)
{
	_parser.SetCommandType(commandType);

	va_list argptr;
	va_start(argptr, command);

	FixedString200 buffer;
	buffer.appendFormatV(command, argptr);
	_currentCommand = buffer;
	_logger.LogAt(F(" => %s"), buffer.c_str());
	_serial.println(buffer.c_str());

	va_end(argptr);
}
AtResultType SimcomAtCommands::GetOperatorName(FixedStringBase &operatorName, bool returnImsi)
{	
	SendAt_P(AtCommand::Cops, F("AT+COPS?"));
	_parserContext.OperatorName = &operatorName;

	const auto result = PopCommandResult();
	if (result != AtResultType::Success)
	{
		return result;
	}

	if (_parserContext.IsOperatorNameReturnedInImsiFormat == returnImsi)
	{
		return result;
	}

	const auto operatorFormat = returnImsi ? 2 : 0;	
	GenericAt(AT_DEFAULT_TIMEOUT, F("AT+COPS=3,%d"), operatorFormat);
	SendAt_P(AtCommand::Cops, F("AT+COPS?"));
	return PopCommandResult();
}
AtResultType SimcomAtCommands::FlightModeOn()
{
	return GenericAt(10000, F("AT+CFUN=0"));
}
AtResultType SimcomAtCommands::FlightModeOff()
{
	return GenericAt(10000, F("AT+CFUN=1"));
}
AtResultType SimcomAtCommands::SetRegistrationMode(RegistrationMode mode, const char *operatorName)
{
	const auto operatorFormat = _parserContext.IsOperatorNameReturnedInImsiFormat ? 2 : 0;
	// don't need to use AtCommand::Cops here, AT+COPS write variant returns OK/ERROR
	SendAt_P(AtCommand::Generic, F("AT+COPS=%d,%d,\"%s\""), mode, operatorFormat, operatorName);
	return PopCommandResult(120000);
}

AtResultType SimcomAtCommands::GetSignalQuality(int16_t& signalQuality)
{	
	_parserContext.CsqSignalQuality = &signalQuality;
	SendAt_P(AtCommand::Csq, F("AT+CSQ"));
	return PopCommandResult();
}

AtResultType SimcomAtCommands::GetBatteryStatus(BatteryStatus &batteryStatus)
{	
	_parserContext.BatteryInfo = &batteryStatus;
	SendAt_P(AtCommand::Cbc,F("AT+CBC"));
	return PopCommandResult();
}

AtResultType SimcomAtCommands::GetIpState(SimcomIpState &ipState)
{	
	_parserContext.IpState = &ipState;
	SendAt_P(AtCommand::Cipstatus, F("AT+CIPSTATUS"));
	return PopCommandResult();	
}

AtResultType SimcomAtCommands::GetIpAddress(GsmIp& ipAddress)
{	
	_parserContext.IpAddress = &ipAddress;
	SendAt_P(AtCommand::Cifsr, F("AT+CIFSR;E1"));
	return PopCommandResult();
}

AtResultType SimcomAtCommands::GetRxMode(bool& isRxManual)
{	
	SendAt_P(AtCommand::CipRxGet, F("AT+CIPRXGET?"));
	const auto result = PopCommandResult();
	if (result == AtResultType::Success)
	{
		isRxManual = _parserContext.IsRxManual;
	}
	return result;
}

AtResultType SimcomAtCommands::SetRxMode(bool isRxManual)
{	
	SendAt_P(AtCommand::Generic, F("AT+CIPRXGET=%d"), isRxManual ? 1 : 0);
	return PopCommandResult();
}

AtResultType SimcomAtCommands::GetCipmux(bool& cipmux)
{	
	SendAt_P(AtCommand::Cipmux, F("AT+CIPMUX?"));
	const auto result = PopCommandResult();
	if (result == AtResultType::Success)
	{
		cipmux = _parserContext.Cipmux;
	}
	return result;
}

AtResultType SimcomAtCommands::SetCipmux(bool cipmux)
{	
	SendAt_P(AtCommand::Generic, F("AT+CIPMUX=%d"), cipmux ? 1 : 0);
	_parserContext.Cipmux = cipmux;
	return PopCommandResult();	
}

AtResultType SimcomAtCommands::GetCipQuickSend(bool& cipqsend)
{
	SendAt_P(AtCommand::CipQsendQuery, F("AT+CIPQSEND?"));
	const auto result = PopCommandResult();
	if (result == AtResultType::Success)
	{
		cipqsend = _parserContext.CipQSend;
	}
	return result;
}

AtResultType SimcomAtCommands::SetSipQuickSend(bool cipqsend)
{
	SendAt_P(AtCommand::Generic, F("AT+CIPQSEND=%d"), cipqsend ? 1: 0);
	return PopCommandResult();
}

AtResultType SimcomAtCommands::AttachGprs()
{	
	SendAt_P(AtCommand::Generic, F("AT+CIICR"));
	return PopCommandResult(60000);
}

AtResultType SimcomAtCommands::PopCommandResult()
{
	return PopCommandResult(AT_DEFAULT_TIMEOUT);
}
AtResultType SimcomAtCommands::PopCommandResult(int timeout)
{
	const unsigned long start = millis();
	while(_parser.commandReady == false && (millis()-start) < (unsigned long)timeout)
	{
		if(_serial.available())
		{
			char c = _serial.read();
			_parser.FeedChar(c);
		}
	}

	const auto commandResult = _parser.GetAtResultType();
	const auto elapsedMs = millis() - start;	
	_logger.LogAt(F("    -- %d ms --"), elapsedMs);
	if (commandResult == AtResultType::Timeout)
	{
		_logger.Log(F("                      --- !!! '%s' - TIMEOUT!!! ---      "), _currentCommand.c_str(), elapsedMs);
	}
	if (commandResult == AtResultType::Error)
	{
		_logger.Log(F("                      --- !!! '%s' - ERROR!!! ---      "), _currentCommand.c_str(), elapsedMs);
	}
	return commandResult;
}
/*
Disables/enables echo on serial port
*/
AtResultType SimcomAtCommands::SetEcho(bool echoEnabled)
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

AtResultType SimcomAtCommands::SetTransparentMode(bool transparentMode)
{	
	SendAt_P(AtCommand::Generic, F("AT+CIPMODE=%d"), transparentMode ? 1:0);
	return PopCommandResult();
}

AtResultType SimcomAtCommands::SetApn(const char *apnName, const char *username,const char *password )
{	
	SendAt_P(AtCommand::Generic, F("AT+CSTT=\"%s\",\"%s\",\"%s\""), apnName, username, password);
	return PopCommandResult();
}

AtResultType SimcomAtCommands::At()
{	
	SendAt_P(AtCommand::Generic, F("AT"));
	return PopCommandResult(30);
}

void SimcomAtCommands::OnDataReceived(DataReceivedCallback onDataReceived)
{
	_parser.OnDataReceived(onDataReceived);
}

AtResultType SimcomAtCommands::SetBaudRate(uint32_t baud)
{	
	SendAt_P(AtCommand::Generic, F("AT+IPR=%d"), baud);
	return PopCommandResult();
}
bool SimcomAtCommands::EnsureModemConnected(long requestedBaudRate)
{	
	auto atResult = At();

	int n = 1;
	if (_currentBaudRate != 0)
	{
		while (atResult != AtResultType::Success && n-- > 0)
		{
			delay(20);
			atResult = At();
		}
		if (atResult == AtResultType::Success || atResult == AtResultType::Error)
		{
			return true;
		}
	}
	_currentBaudRate = FindCurrentBaudRate();
	if (_currentBaudRate == 0)
	{
		return false;
	}

	_logger.Log(F("Found baud rate = %d"), _currentBaudRate);
	
	if (SetBaudRate(requestedBaudRate) != AtResultType::Success)
	{
		_logger.Log(F("Failed to update baud rate to %d"), requestedBaudRate);
		return false;
	}
	_currentBaudRate = requestedBaudRate;
	_logger.Log(F("Updated baud rate to = %d"), _currentBaudRate);

	At();

	if (SetEcho(true) != AtResultType::Success)
	{
		_logger.Log(F("Failed to set echo"));
		return false;	
	}
	_parser.ResetUartGarbageDetected();
	return true;	
}
AtResultType SimcomAtCommands::GetImei(FixedString20 &imei)
{	
	_parserContext.Imei = &imei;
	SendAt_P(AtCommand::Gsn, F("AT+GSN"));
	return PopCommandResult();
}

void SimcomAtCommands::wait(uint64_t ms)
{
	const unsigned long start = millis();
	while ((millis() - start) <= ms)
	{
		if (_serial.available())
		{
			auto c = _serial.read();
			_parser.FeedChar(c);
		}
	}
}

bool SimcomAtCommands::GarbageOnSerialDetected()
{
	return _parser.GarbageOnSerialDetected();
}

AtResultType SimcomAtCommands::SendSms(char *number, char *message)
{	
	SendAt_P(AtCommand::Generic, F("AT+CMGS=\"%s\""), number);

	const uint64_t start = millis();
	// wait for >
	while (_serial.read() != '>')
		if (millis() - start > 200)
			return AtResultType::Error;
	_serial.print(message);
	_serial.print('\x1a');
	return PopCommandResult();
}
AtResultType SimcomAtCommands::SendUssdWaitResponse(char *ussd, FixedString150& response)
{
	_parserContext.UssdResponse = &response;
	SendAt_P(AtCommand::Cusd, F("AT+CUSD=1,\"%s\""), ussd);
	return PopCommandResult(10000);
}

int SimcomAtCommands::FindCurrentBaudRate()
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

AtResultType SimcomAtCommands::Cipshut()
{	
	SendAt_P(AtCommand::Cipshut, F("AT+CIPSHUT"));
	return PopCommandResult(20000);
}

AtResultType SimcomAtCommands::Call(char *number)
{
	SendAt_P(AtCommand::Generic, F("ATD%s;"), number);
	return PopCommandResult();
}

AtResultType SimcomAtCommands::GetIncomingCall(IncomingCallInfo & callInfo)
{
	_parserContext.CallInfo = &callInfo;
	SendAt_P(AtCommand::Clcc, F("AT+CLCC"));
	const auto result = PopCommandResult();
	return result;
}

AtResultType SimcomAtCommands::Shutdown()
{	
	SendAt_P(AtCommand::Generic, F("AT+CPOWD=0"));
	return PopCommandResult();
}


AtResultType SimcomAtCommands::BeginConnect(ProtocolType protocol, uint8_t mux, const char *address, int port)
{	
	_logger.Log(F("BeginConnect %s:%u"), address, port);

	SendAt_P(AtCommand::Generic, 
		F("AT+CIPSTART=%d,\"%s\",\"%s\",\"%d\""),
		mux, ProtocolToStr(protocol), address, port);	
	return PopCommandResult(60000);
}

AtResultType SimcomAtCommands::Read(int mux, FixedStringBase& outputBuffer)
{
	_parserContext.CipRxGetBuffer = &outputBuffer;
	SendAt_P(AtCommand::CipRxGetRead,F("AT+CIPRXGET=2,%d,%d"), mux, outputBuffer.capacity());
	return PopCommandResult();
}

AtResultType SimcomAtCommands::Send(int mux, FixedStringBase& data, uint16_t &sentBytes)
{
	sentBytes = 0;
	_parserContext.CipsendBuffer = &data;
	_parserContext.CipsendState = CipsendStateType::WaitingForPrompt;
	_parserContext.CipsendSentBytes = &sentBytes;
	SendAt_P(AtCommand::CipSend, F("AT+CIPSEND=%d,%d"), mux, data.length());
	return PopCommandResult();
}

AtResultType SimcomAtCommands::CloseConnection(uint8_t mux)
{	
	SendAt_P(AtCommand::Cipclose, F("AT+CIPCLOSE=%d"), mux);
	return PopCommandResult();
}

AtResultType SimcomAtCommands::GetConnectionInfo(uint8_t mux, ConnectionInfo &connectionInfo)
{	
	_parserContext.CurrentConnectionInfo = &connectionInfo;
	SendAt_P(AtCommand::CipstatusSingleConnection, F("AT+CIPSTATUS=%d"), mux);
	return PopCommandResult();
}







































