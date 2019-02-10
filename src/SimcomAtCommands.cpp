#include "SimcomAtCommands.h"
#include "GsmLibHelpers.h"

SimcomAtCommands::SimcomAtCommands(Stream& serial, UpdateBaudRateCallback updateBaudRateCallback, SetDtrCallback setDtrCallback, CpuSleepCallback cpuSleepCallback) :
_serial(serial),
_updateBaudRateCallback(updateBaudRateCallback),
_cpuSleepCallback(cpuSleepCallback),
_currentBaudRate(0),
_setDtrCallback(setDtrCallback),
_parser(_parserContext, _logger, serial, _currentCommand),
IsAsync(false),
_isInSleepMode(false),
_lastIncomingByteTime(0)
{
}

AtResultType SimcomAtCommands::PopCommandResult(bool ensureDelay)
{
	return PopCommandResult(ensureDelay, AT_DEFAULT_TIMEOUT);
}

void SimcomAtCommands::ReadCharAndFeedParser()
{
	if (!_serial.available())
	{
		return;
	}
	auto c = _serial.read();
	_parser.FeedChar(c);
	_lastIncomingByteTime = millis();	
}
void SimcomAtCommands::ReadCharAndIgnore()
{
	if (!_serial.available())
	{
		return;
	}
	auto c = _serial.read();
	_lastIncomingByteTime = millis();
}
AtResultType SimcomAtCommands::PopCommandResult(bool ensureDelay, uint64_t timeout)
{
	if (ensureDelay)
	{
		auto before = millis();
		while (millis() - _lastIncomingByteTime < 100)
		{
			ReadCharAndFeedParser();
		}
		auto waitTime = millis() - before;
		_logger.Log(F("Waited %u ms"), waitTime);
	}
	_serial.print(_currentCommand.c_str());
	_serial.print("\r\n");
	_serial.flush();
	_logger.LogAt(F(" => %s"), _currentCommand.c_str());

	const unsigned long start = millis();
	while (_parser.commandReady == false && (millis() - start) < timeout)
	{
		ReadCharAndFeedParser();		
	}
	const auto commandResult = _parser.GetAtResultType();
	const auto elapsedMs = millis() - start;
	
	if (commandResult == AtResultType::Success)
	{
		_logger.LogAt(F("  SUCCESS  -- %d ms --"), elapsedMs);
	}	
    else if (commandResult == AtResultType::Timeout)
	{
		TimeoutedCommand = _currentCommand;
		_logger.Log(F("                      --- TIMEOUT executing '%s', elapsed %d ms ---"), _currentCommand.c_str(), elapsedMs);
	}
	else if (commandResult == AtResultType::Error)
	{
		_logger.Log(F("                      --- ERROR executing '%s', elapsed %d ms ---"), _currentCommand.c_str(), elapsedMs);
	}
	return commandResult;
}
void SimcomAtCommands::wait(uint64_t ms)
{
	const unsigned long start = millis();
	while ((millis() - start) <= ms)
	{
		ReadCharAndFeedParser();
	}
}

AtResultType SimcomAtCommands::GenericAt(uint64_t timeout, const __FlashStringHelper* command, ...)
{	
	_parser.SetCommandType(AtCommand::Generic);
	va_list argptr;
	va_start(argptr, command);

	_currentCommand.clear();
	_currentCommand.appendFormatV(command, argptr);

	const auto result = PopCommandResult(false, timeout);
	va_end(argptr);	
	return result;
}
void SimcomAtCommands::SendAt_P(AtCommand commandType, const __FlashStringHelper* command, ...)
{
	_parser.SetCommandType(commandType);

	va_list argptr;
	va_start(argptr, command);
	FixedString200 buffer;

	_currentCommand.clear();
	_currentCommand.appendFormatV(command, argptr);

	va_end(argptr);
}
void SimcomAtCommands::SendAt_P(AtCommand commandType, bool expectEcho, const __FlashStringHelper* command, ...)
{
	_parser.SetCommandType(commandType, expectEcho);

	va_list argptr;
	va_start(argptr, command);

	_currentCommand.clear();
	_currentCommand.appendFormatV(command, argptr);

	va_end(argptr);
}

AtResultType SimcomAtCommands::At(uint32_t timeout, bool expectEcho)
{
	SendAt_P(AtCommand::Generic, expectEcho, F("AT"));
	return PopCommandResult(false, timeout);
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
	SendAt_P(AtCommand::Creg, F("AT+CREG?"));

	const auto result = PopCommandResult();
	if (result == AtResultType::Success)
	{
		registrationStatus = _parserContext.RegistrationStatus;
	}
	return result;
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

	if (operatorName == nullptr)
	{
		SendAt_P(AtCommand::Generic, F("AT+COPS=%d"), mode);
	}
	else
	{
		SendAt_P(AtCommand::Generic, F("AT+COPS=%d,%d,\"%s\""), mode, operatorFormat, operatorName);
	}
	return PopCommandResult(false, 120000u);
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
	return PopCommandResult(false, 60000u);
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

AtResultType SimcomAtCommands::SetBaudRate(uint32_t baud)
{	
	SendAt_P(AtCommand::Generic, F("AT+IPR=%d"), baud);
	return PopCommandResult();
}
bool SimcomAtCommands::EnsureModemConnected(uint64_t requestedBaudRate)
{
	_currentBaudRate = FindCurrentBaudRate();
	if (_currentBaudRate == 0)
	{
		return false;
	}

	_logger.Log(F("Found baud rate = %d"), _currentBaudRate);
	
	if (_currentBaudRate != requestedBaudRate)
	{
		if (SetBaudRate(requestedBaudRate) != AtResultType::Success)
		{
			_logger.Log(F("Failed to update baud rate to %d"), requestedBaudRate);
			return false;
		}
		_currentBaudRate = requestedBaudRate;
		_logger.Log(F("Updated baud rate to = %d"), _currentBaudRate);
	}

	At();

	if (SetEcho(true) != AtResultType::Success)
	{
		_logger.Log(F("Failed to set echo"));
		return false;	
	}
	_parser.ResetUartGarbageDetected();
	return true;	
}

uint64_t SimcomAtCommands::FindCurrentBaudRate()
{
	if (_updateBaudRateCallback == nullptr)
	{
		_logger.Log(F("Change baud rate callback is null"));
		return 0;
	}

	//garbage detection is disabled as change baud rate might result in 
	//receiving couple of garbage characters, safe to ignore
	_parser.IsGarbageDetectionActive = false;
	int i = 0;
	uint64_t baudRate = 0;
	AtResultType commandResult;
	do
	{
		auto baudRateToTry = _defaultBaudRates[i];
		_logger.Log(F("Trying baud rate: %d"), baudRateToTry);
		_updateBaudRateCallback(baudRateToTry);
		commandResult = At();
		if (commandResult == AtResultType::Success)
		{
			baudRate = baudRateToTry;
		}

		i++;
	} while (_defaultBaudRates[i] != 0 && commandResult != AtResultType::Success);
	_parser.IsGarbageDetectionActive = true;
	return baudRate;
}

AtResultType SimcomAtCommands::GetImei(FixedString20 &imei)
{	
	_parserContext.Imei = &imei;
	SendAt_P(AtCommand::Gsn, F("AT+GSN"));
	return PopCommandResult();
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
	return PopCommandResult(false, 10000u);
}

AtResultType SimcomAtCommands::Cipshut()
{	
	SendAt_P(AtCommand::Cipshut, F("AT+CIPSHUT"));
	return PopCommandResult(false, 20000u);
}

bool SimcomAtCommands::SetDtr(bool value)
{
	if (_setDtrCallback == nullptr)
	{
		return false;
	}	
	if (value)
	{
		_logger.Log(F("Pulling DTR up"));
	}
	else
	{
		_logger.Log(F("Pulling DTR down"));
	}
	return _setDtrCallback(value);
}

AtResultType SimcomAtCommands::Call(char *number)
{
	SendAt_P(AtCommand::Generic, F("ATD%s;"), number);
	return PopCommandResult();
}

AtResultType SimcomAtCommands::HangUp()
{
	SendAt_P(AtCommand::Generic, F("ATH"));
	return PopCommandResult();
}

AtResultType SimcomAtCommands::GetIncomingCall(IncomingCallInfo & callInfo)
{
	callInfo.HasIncomingCall = false;
	callInfo.CallerNumber.clear();
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

AtResultType SimcomAtCommands::EnableNetlight(bool enable)
{
	SendAt_P(AtCommand::Generic, F("AT+CNETLIGHT=%d"), enable ? 1 : 0);
	return PopCommandResult();
}
AtResultType SimcomAtCommands::GetTemperature(float& temperature)
{
	_parserContext.Temperature = &temperature;
	SendAt_P(AtCommand::Cmte, F("AT+CMTE?"));
	return PopCommandResult();
}
AtResultType SimcomAtCommands::BeginConnect(ProtocolType protocol, uint8_t mux, const char *address, int port)
{	
	_logger.Log(F("BeginConnect %s:%u"), address, port);

	SendAt_P(AtCommand::Generic, 
		F("AT+CIPSTART=%d,\"%s\",\"%s\",\"%d\""),
		mux, ProtocolToStr(protocol), address, port);	
	return PopCommandResult(false, 60000u);
}

AtResultType SimcomAtCommands::Read(int mux, FixedStringBase& outputBuffer, uint16_t& availableBytes)
{
	_parserContext.CipRxGetBuffer = &outputBuffer;
	_parserContext.CiprxGetAvailableBytes = &availableBytes;
	SendAt_P(AtCommand::CipRxGetRead,F("AT+CIPRXGET=2,%d,%d"), mux, outputBuffer.capacity());
	return PopCommandResult(false);
}

AtResultType SimcomAtCommands::Send(int mux, FixedStringBase & data, uint16_t index, uint16_t length, uint16_t & sentBytes)
{
	sentBytes = 0;
	_parserContext.CipsendBuffer = &data;
	_parserContext.CipsendState = CipsendStateType::WaitingForPrompt;
	_parserContext.CipsendDataIndex = index;
	_parserContext.CipsendDataLength = length;
	_parserContext.CipsendSentBytes = &sentBytes;
	SendAt_P(AtCommand::CipSend, F("AT+CIPSEND=%d,%d"), mux, data.length());
	return PopCommandResult(false);
}

AtResultType SimcomAtCommands::Send(int mux, FixedStringBase& data, uint16_t &sentBytes)
{
	return Send(mux, data, 0, data.length(), sentBytes);
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
void SimcomAtCommands::OnMuxEvent(void* ctx, MuxEventHandler muxEventHandler)
{
	_parser.OnMuxEvent(ctx, muxEventHandler);
}
void SimcomAtCommands::OnCipstatusInfo(void* ctx, MuxCipstatusInfoHandler muxCipstatusHandler)
{
	_parser.OnMuxCipstatusInfo(ctx, muxCipstatusHandler);
}

void SimcomAtCommands::OnGsmModuleEvent(void * ctx, OnGsmModuleEventHandler gsmModuleEventHandler)
{
	_parser.OnGsmModuleEvent(ctx, gsmModuleEventHandler);
}

AtResultType SimcomAtCommands::EnterSleepMode()
{
	if (!SetDtr(true))
	{
		return AtResultType::Error;
	}

	SendAt_P(AtCommand::Generic, F("AT+CSCLK=1"));
	auto result = PopCommandResult();
	if (result == AtResultType::Success)
	{
		_isInSleepMode = true;
	}
	return result;
}

AtResultType SimcomAtCommands::ExitSleepMode()
{
	_logger.Log(F("Exiting sleep mode"));
	if (!SetDtr(false))
	{
		return AtResultType::Error;
	}
	
	delay(100);
	
	int n = 0;
	while (At(150u, false) != AtResultType::Success)
	{	
		if (n % 2 == 0 && n >= 2)
		{
			SetDtr(true);
			delay(20);
			SetDtr(false);
			delay(100);
		}
		if (n == 40)
		{
			return AtResultType::Timeout;
		}
		n++;
	}

	for (int i = 0; i < 5; i++)
	{
		wait(1);
		SendAt_P(AtCommand::Generic, F("AT+CSCLK=0"));
		auto result = PopCommandResult();
		if (result == AtResultType::Success)
		{
			_logger.Log(F("Sucessfully executed AT+CSCLK=0, i = %d"), i);
			for (int j = 0; j < 3; j++)
			{
				GsmRegistrationState regState;
				wait(1);
				if (GetRegistrationStatus(regState) == AtResultType::Success)
				{
					_logger.Log(F("Sucessfully executed AT+CREG j = %d"), j);

					_isInSleepMode = false;
					return AtResultType::Success;
				}
			}
		}
		wait(100);
	}

	
	return AtResultType::Timeout;
}

bool SimcomAtCommands::IsInSleepMode()
{
	return _isInSleepMode;
}

bool SimcomAtCommands::CpuSleep(uint64_t millis)
{
	if (_cpuSleepCallback == nullptr)
	{
		return false;
	}

	_logger.Log(F("Entering CPU sleep"));
	_logger.Flush();
	_cpuSleepCallback(millis);
	_logger.Log(F("Wake up from CPU sleep"));	
	return true;
}



































