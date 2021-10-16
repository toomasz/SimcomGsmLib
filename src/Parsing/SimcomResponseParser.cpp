#include "SimcomResponseParser.h"

#include "../GsmLibHelpers.h"
#include "ParsingHelpers.h"

SimcomResponseParser::SimcomResponseParser(ParserContext& parserContext, GsmLogger& logger, Stream& serial, FixedStringBase &currentCommandStr):
_logger(logger),
_parserContext(parserContext),
_garbageOnSerialDetected(false),
_serial(serial),
_promptSequenceDetector("> "),
_currentCommandStr(currentCommandStr),
_onMuxEvent(nullptr),
_onMuxEventCtx(nullptr),
_onMuxCipstatusInfo(nullptr),
_onMuxCipstatusInfoCtx(nullptr),
_onGsmModuleEvent(nullptr),
_onGsmModuleEventCtx(nullptr),
commandReady(false),
IsGarbageDetectionActive(true)
{
	_currentCommand = AtCommand::Generic;
	lineParserState = LineState::PARSER_INITIAL;
	_state = ParserState::Timeout;
}

AtResultType SimcomResponseParser::GetAtResultType()
{
	switch (_state)
	{
		case ParserState::Success:
			return AtResultType::Success;
		case ParserState::Error:
			return AtResultType::Error;

		case ParserState::PartialSuccess:
		case ParserState::PartialError:
		case ParserState::Timeout:
		case ParserState::None:
		default:
			return AtResultType::Timeout;
	}
}


/* processes character read from serial port of gsm module */
void SimcomResponseParser::FeedChar(char c)
{	
	if (_state != ParserState::WaitingForEcho)
	{
		if (_currentCommand == AtCommand::CipSend)
		{
			if (_parserContext.CipsendState == CipsendStateType::WaitingForPrompt)
			{
				if (_promptSequenceDetector.NextChar(c))
				{
					_parserContext.CipsendState = CipsendStateType::WaitingForDataEcho;
					_response.clear();
					_logger.Log(F("Writing %d b of data"), _parserContext.CipsendDataLength);

					auto dataPtr = _parserContext.CipsendBuffer->c_str() + _parserContext.CipsendDataIndex;
					auto dataLength = _parserContext.CipsendDataLength;

					_serial.write(dataPtr, dataLength);
					_parserContext.CipsendDataEchoDetector.SetSequence(dataPtr, dataLength);
					return;
				}
			}
			if (_parserContext.CipsendState == CipsendStateType::WaitingForDataEcho)
			{
				if (_parserContext.CipsendDataEchoDetector.NextChar(c))
				{
					_parserContext.CipsendState = CipsendStateType::WaitingForDataAccept;
				}
				return;
			}
		}
	}
	if (_parserContext.CiprxGetLeftBytesToRead > 0)
	{
		_parserContext.CipRxGetBuffer->append(c);
		_parserContext.CiprxGetLeftBytesToRead--;
		return;
	}	
	LineState prevState = lineParserState;

	lineParserState = StateTransition(c);

	if(prevState  == LineState::PARSER_INITIAL || prevState == LineState::PARSER_LF || prevState == LineState::PARSER_LINE)
	{		
		if (lineParserState == LineState::PARSER_LINE)
		{
			if (_response.freeBytes() > 0)
			{
				_response.append(c);
			}
		}
	}
	// line -> delimiter
	if ((prevState == LineState::PARSER_LINE || prevState == LineState::PARSER_CR) && (lineParserState == LineState::PARSER_LF))
	{
		if (_response.length() == 0)
		{
			return;
		}

		_logger.LogAt(F("    <= %s"), (char*)_response.c_str());

		auto isUnsolicited = ParseUnsolicited(_response);

		if (isUnsolicited)
		{
			_response.clear();
			return;
		}
		ParserState parseResult = ParseLine();

		// if error or or success
		if (parseResult == ParserState::Success || parseResult == ParserState::Error)
		{
			commandReady = true;
			_state = parseResult;
		}
		
		// if command not parsed yet
		else if (parseResult == ParserState::None && !_response.equals(_currentCommandStr.c_str()))
		{
			if (ParsingHelpers::CheckIfLineContainsGarbage(_response))
			{
				if (IsGarbageDetectionActive)
				{
					_garbageOnSerialDetected = true;
					_logger.Log(F(" Garbage detected(%d b): "), _response.length());
				}

			}
			else
			{				
				_logger.Log(F( "Unknown response (%d b): "), _response.length());				
			}

			FixedString256 printableLine;
			BinaryToString(_response, printableLine);
			_logger.Log(F(" '%s'"), printableLine.c_str());
			// do nothing, do not change _state to none
		}
		else
		{
			_state = parseResult;
		}
		
		_response.clear();
	}

}

/* returns true if current line is error: ERROR, CME ERROR etc*/
bool SimcomResponseParser::IsErrorLine()
{
	if (_response == F("NO CARRIER"))
	{
		return true;
	}
	if (_response == F("ERROR"))
	{
		return true;
	}
	if (_response.startsWith(F("+CME ERROR:")))
	{
		return true;
	}	
	return false;
}
/* returns true if current line is OK*/
bool SimcomResponseParser::IsOkLine()
{
	return _response == F("OK");
}
bool SimcomResponseParser::IsCurrentLineAllInHexChars()
{
    if(_response.length() == 0)
    {
        return false;
    }
    for(int i =0; i < _response.length(); i++)
    {
        if(!isxdigit(_response[i]))
        {
            return false;
        }
    }
    return true;
}
bool SimcomResponseParser::ParseUnsolicited(FixedStringBase& line)
{
	if(line.equals(F("+CIPRXGET: 1,0")))
	{
		return true;
	}
	if (line.equals(F("SMS Ready")))
	{
		return true;
	}
	if (line.equals(F("Call Ready")))
	{
		_logger.Log(F("Call ready"));
		return true;
	}
	
	if (line.equals(F("OVER-VOLTAGE WARNNING")))
	{
		_logger.Log(F(" Over voltage warning  !!!"));
		RaiseGsmModuleEvent(GsmModuleEventType::OverVoltageWarning);
		return true;
	}
	if (line.equals(F("OVER-VOLTAGE POWER DOWN")))
	{
		_logger.Log(F(" Over voltage power down  !!!"));
		RaiseGsmModuleEvent(GsmModuleEventType::OverVoltagePowerDown);
		return true;
	}
	if (line.equals(F("UNDER-VOLTAGE WARNNING")))
	{
		_logger.Log(F(" Under voltage warning !!!"));
		RaiseGsmModuleEvent(GsmModuleEventType::UnderVoltageWarining);
		return true;
	}
	if (line.equals(F("UNDER-VOLTAGE POWER DOWN")))
	{
		_logger.Log(F(" Over voltage power down  !!!"));
		RaiseGsmModuleEvent(GsmModuleEventType::UnderVoltagePowerDown);
		return true;
	}

	DelimParser parser(line);

	uint8_t mux;

	if (parser.NextNum(mux))
	{
		FixedString64 str;
		if (mux <= 5)
		{
			if (parser.NextString(str))
			{
				_logger.Log(F("Mux: %d, event = %s"), mux, str.c_str());
				if (_onMuxEvent != nullptr)
				{
					_response.clear();
					return _onMuxEvent(_onMuxEventCtx, mux, str);
				}
				return true;
			}
		}
	}
	return false;
}

ParserState SimcomResponseParser::ParseLine()
{
	if (_state == ParserState::WaitingForEcho)
	{
		if (_response.equals(_currentCommandStr))
		{		
			return ParserState::Timeout;
		}
		return ParserState::None;
	}

	DelimParser parser(_response);


	if (_currentCommand == AtCommand::Cpin)
	{
		if (IsErrorLine())
		{
			_parserContext.SimStatus = SimState::NotInserted;
			return ParserState::Success;
		}
		if (_response == F("+CPIN: READY"))
		{
			_parserContext.SimStatus = SimState::Ok;
			return ParserState::PartialSuccess;
		}
		if (_response == F("+CPIN: SIM PIN"))
		{
			_parserContext.SimStatus = SimState::Locked;
			return ParserState::PartialSuccess;
		}
		if (_response == F("+CPIN: SIM PUK"))
		{
			_parserContext.SimStatus = SimState::Locked;
			return ParserState::PartialSuccess;
		}
	}

	if(_currentCommand == AtCommand::Generic)
	{
		if (IsErrorLine())
		{
			return ParserState::Error;
		}
		if (IsOkLine())
		{
			return ParserState::Success;
		}
	}
	
	if(_currentCommand == AtCommand::Cipstatus)
	{
		static uint8_t internalState = 0;
		// Cipstatus returns OK first, then IP STATE: xxxx
		if (IsOkLine())
		{
			internalState = 0;
			return ParserState::PartialSuccess;
		}
		if (_state == ParserState::PartialSuccess)
		{
			if (internalState == 0)
			{
				if (ParsingHelpers::ParseIpStatus(_response.c_str(), *_parserContext.IpState))
				{
					if (!_parserContext.Cipmux)
					{
						return ParserState::Success;
					}
					internalState = 1;
					return ParserState::PartialSuccess;
				}
			}
			if (internalState >= 1)
			{
				if(parser.StartsWith(F("C: ")))
				{ 
					ConnectionInfo info;
					if (ParsingHelpers::ParseSocketStatusLine(parser, info))
					{
						if (_onMuxCipstatusInfo != nullptr)
						{
							_onMuxCipstatusInfo(_onMuxCipstatusInfoCtx, info);
						}
					}
					internalState++;
					if (internalState == 7)
					{
						return ParserState::Success;
					}
					return ParserState::PartialSuccess;
				}
			}			
		}
	}
	if (_currentCommand == AtCommand::CipstatusSingleConnection)
	{
		if (parser.StartsWith(F("+CIPSTATUS: ")))
		{
			if (ParsingHelpers::ParseSocketStatusLine(parser, *_parserContext.CurrentConnectionInfo, true))
			{
				return ParserState::PartialSuccess;
			}
			else
			{
				return ParserState::PartialError;
			}			
		}
	}

	if (_currentCommand == AtCommand::CipRxGetRead)
	{
		if (parser.StartsWith(F("+CIPRXGET: ")))
		{
			uint8_t mode;
			uint8_t mux;
			uint16_t dataSize;
			uint16_t dataLeft;
			if (parser.NextNum(mode) &&
				parser.NextNum(mux) &&
				parser.NextNum(dataSize) && 
				parser.NextNum(dataLeft))
			{
				_parserContext.CiprxGetLeftBytesToRead = dataSize;
				*_parserContext.CiprxGetAvailableBytes = dataLeft;
				return ParserState::PartialSuccess;				 
			}
		}
	}

	if(_currentCommand == AtCommand::Csq)
	{
		//+CSQ: 17,0
		if(parser.StartsWith(F("+CSQ: ")))
		{
			uint16_t signalQuality;
			uint16_t signalStrength;

			if (parser.NextNum(signalQuality) && parser.NextNum(signalStrength))
			{
				*_parserContext.CsqSignalQuality = signalQuality;
				return ParserState::PartialSuccess;
			}
			return ParserState::PartialError;
		}
	}

	if (_currentCommand == AtCommand::Cbc)
	{
		if (parser.StartsWith(F("+CBC: ")))
		{
			uint16_t batteryPercent;
			uint16_t mVbatteryVoltage;

			if (!parser.Skip(1) || 
				!parser.NextNum(batteryPercent) ||
				!parser.NextNum(mVbatteryVoltage))
			{
				return ParserState::PartialError;
			}
			_parserContext.BatteryInfo->Voltage = mVbatteryVoltage / 1000.0;
			_parserContext.BatteryInfo->Percent = batteryPercent;
			return ParserState::PartialSuccess;			
		}
	}

	if(_currentCommand == AtCommand::Cifsr)
	{
		GsmIp ip;
		if (ParsingHelpers::ParseIpAddress(_response, ip))
		{
			*_parserContext.IpAddress = ip;
			return ParserState::PartialSuccess;
		}
	}

	if (_currentCommand == AtCommand::Clcc)
	{
		if (parser.StartsWith(F("+CLCC: ")))
		{
			if (!parser.Skip(5))
			{
				return ParserState::PartialError;
			}
			FixedString32 number;			
			if (!parser.NextString(number))
			{
				return ParserState::PartialError;
			}
			_parserContext.CallInfo->CallerNumber = number;
			_parserContext.CallInfo->HasIncomingCall = true;	
			return ParserState::PartialSuccess;
		}
		// CLCC can return no records so it's ok
		if (IsOkLine())
		{
			return ParserState::Success;
		}
	}

	if(_currentCommand == AtCommand::Cipstart)
	{
		if (_response == F("CONNECT"))
		{
			return ParserState::Success;
		}
		if (_response == F("CONNECT FAIL") || _response == F("+PDP: DEACT"))
		{
			return ParserState::Error;
		}
	}
	if(_currentCommand == AtCommand::Cipshut)
	{
		if (_response.equals(F("SHUT OK")))
		{
			return ParserState::Success;
		}
	}
	if(_currentCommand == AtCommand::Cipclose)
	{
		if (_response.endsWith(F("CLOSE OK")))
		{
			return ParserState::Success;
		}
	}
	if(_currentCommand == AtCommand::Cops)
	{
		if(parser.StartsWith(F("+COPS: ")))
		{				
			uint16_t operatorNameFormat;

			if (!parser.NextNum(_parserContext.operatorSelectionMode) ||
				!parser.NextNum(operatorNameFormat) || 
				!parser.NextString(*_parserContext.OperatorName))
			{
				return ParserState::PartialError;
			}

			_parserContext.IsOperatorNameReturnedInImsiFormat = operatorNameFormat == 2;
			return ParserState::PartialSuccess;			
		}
	}
	if(_currentCommand == AtCommand::Gsn)
	{
		auto imeiValid = ParsingHelpers::IsImeiValid(_response);
		if (imeiValid)
		{
			*_parserContext.Imei = _response;
			return ParserState::PartialSuccess;
		}
	}
	if (_currentCommand == AtCommand::Cusd)
	{
		if (parser.StartsWith(F("+CUSD: ")))
		{
			uint16_t tmp = 0;
			if (!parser.NextNum(tmp) ||	
				!parser.NextString(*_parserContext.UssdResponse))
			{				
				return ParserState::PartialError;				
			}
			return ParserState::PartialSuccess;
		}
	}
	if (_currentCommand == AtCommand::Cipmux)
	{
		if (parser.StartsWith(F("+CIPMUX: ")))
		{
			uint16_t isEnabled;
			if (!parser.NextNum(isEnabled))
			{
				return ParserState::PartialError;
			}
			_parserContext.Cipmux = isEnabled == 1;
			return ParserState::PartialSuccess;			
		}		
	}
	if(_currentCommand == AtCommand::CipQsendQuery)
	{
		if(parser.StartsWith(F("+CIPQSEND: ")))
		{
			uint16_t isEnabled;
			if (!parser.NextNum(isEnabled))
			{
				return ParserState::PartialError;
			}
			_parserContext.CipQSend = isEnabled == 1;
			return ParserState::PartialSuccess;
		}
	}
	if (_currentCommand == AtCommand::CipRxGet)
	{
		if (parser.StartsWith(F("+CIPRXGET:")))
		{
			uint16_t isEnabled;
			if (!parser.NextNum(isEnabled))
			{
				return ParserState::PartialError;
			}
			_parserContext.IsRxManual = isEnabled == 1;
			return ParserState::PartialSuccess;
		}
	}
	if(_currentCommand == AtCommand::CipSend)
	{
		if (_parserContext.CipsendState == CipsendStateType::WaitingForDataAccept)
		{
			if (parser.StartsWith(F("DATA ACCEPT:")))
			{
				uint16_t sentBytes;
				if (!parser.Skip(1))
				{
					return ParserState::Error;
				}

				if (!parser.NextNum(sentBytes))
				{
					return ParserState::Error;
				}
				if (sentBytes > _parserContext.CipsendDataLength)
				{
					return ParserState::Error;
				}
				*_parserContext.CipsendSentBytes = sentBytes;
				return ParserState::Success;				
			}
			if (_response.endsWith(F("SEND FAIL")))
			{
				_logger.Log(F("CIPSEND failed, SEND FAIL detected"));
				return ParserState::Error;
			}
		}
		if (_parserContext.CipsendState == CipsendStateType::WaitingForPrompt)
		{
			if(IsErrorLine())
			{
				_logger.Log(F("CIPSEND failed, error line detected"));
				return ParserState::Error;
			}			
		}
	}
	if(_currentCommand == AtCommand::Creg)
	{
		// example valid line : +CREG: 2,1,"07E6","D68F"
		if(parser.StartsWith(F("+CREG: ")))
		{

			if (!parser.Skip(1))
			{
				return ParserState::PartialError;
			}
			uint8_t cregRegistrationState;
			if (!parser.NextNum(cregRegistrationState))
			{
				return ParserState::PartialError;
			}

			if (!ParsingHelpers::ParseRegistrationStatus(cregRegistrationState, _parserContext.RegistrationStatus))
			{
				return ParserState::PartialError;
			}
			if (!parser.NextNum(_parserContext.CregLac, false, 16) || !parser.NextNum(_parserContext.CregCellId, false, 16))
			{
				_parserContext.CregLac = 0;
				_parserContext.CregCellId = 0;
			}
			return ParserState::PartialSuccess;
		}	
	}

	if (_currentCommand == AtCommand::Cmte)
	{
		if (parser.StartsWith(F("+CMTE: ")))
		{
			if (!parser.Skip(1))
			{
				return ParserState::PartialError;
			}
			if (!parser.NextFloat(*_parserContext.Temperature))
			{
				return ParserState::PartialError;
			}
			return ParserState::PartialSuccess;
		}
	}

    if (_currentCommand == AtCommand::Cpms)
	{
		if (parser.StartsWith(F("+CPMS: ")))
		{
			if (!parser.Skip(7))
			{
				return ParserState::PartialError;
			}
			if (!parser.NextNum(*_parserContext.lastSmsMessageIndex))
			{
				return ParserState::PartialError;
			}
			return ParserState::PartialSuccess;
		}
	}
    if (_currentCommand == AtCommand::Cmgr)
	{
        if(parser.StartsWith(F("+CMGR: ")))
        {
            return ParserState::PartialSuccess;
        }
        if(IsCurrentLineAllInHexChars())
        {
            *_parserContext.smsMessage = _response;
            return ParserState::PartialSuccess;
        }
	}
	if (IsOkLine())
	{
		if (_state == ParserState::PartialSuccess)
		{
			return ParserState::Success;
		}
		if (_state == ParserState::PartialError)
		{
			return ParserState::Error;
		}
	}
	if (IsErrorLine())
	{
		return ParserState::Error;
	}

	return ParserState::None;
}

void SimcomResponseParser::SetCommandType(AtCommand command, bool expectEcho)
{		
	_currentCommand = command;
	commandReady = false;	
	if (expectEcho)
	{
		_state = ParserState::WaitingForEcho;
	}
	else
	{
		_state = ParserState::Timeout;
	}
}

LineState SimcomResponseParser::StateTransition(char c)
{
	switch (lineParserState)
	{
		case LineState::PARSER_INITIAL:
			if (c == '\r')
			{
				return LineState::PARSER_CR;
			}
			else if (c == '\n')
			{
				return LineState::PARSER_LF;
			}
			else
			{
				return LineState::PARSER_LINE;
			}
		case LineState::PARSER_CR:
			if (c == '\r')
			{
				return LineState::PARSER_CR;
			}
			else if (c == '\n')
			{
				return LineState::PARSER_LF;
			}
			else
			{
				return LineState::PARSER_INITIAL;
			}
		case LineState::PARSER_LF:
			if (c == '\r')
			{
				return LineState::PARSER_CR;
			}
			else if (c == '\n')
			{
				return LineState::PARSER_LF;
			}
			else
			{
				return LineState::PARSER_LINE;
			}
		case LineState::PARSER_LINE:
		default:
			if (c == '\r')
			{
				return LineState::PARSER_CR;
			}
			else if (c == '\n')
			{
				return LineState::PARSER_LF;
			}
			else
			{
				return LineState::PARSER_LINE;
			}
	}

}

void SimcomResponseParser::RaiseGsmModuleEvent(GsmModuleEventType eventType)
{
	if (_onGsmModuleEvent != nullptr)
	{
		_onGsmModuleEvent(_onGsmModuleEventCtx, eventType);
	}
}

bool SimcomResponseParser::GarbageOnSerialDetected()
{
	return _garbageOnSerialDetected;
}

void SimcomResponseParser::ResetUartGarbageDetected()
{
	_garbageOnSerialDetected = false;
}

void SimcomResponseParser::OnMuxEvent(void * ctx, MuxEventHandler onMuxEvent)
{
	_onMuxEventCtx = ctx;
	_onMuxEvent = onMuxEvent;
}

void SimcomResponseParser::OnMuxCipstatusInfo(void * ctx, MuxCipstatusInfoHandler onMuxCipstatusInfo)
{
	_onMuxCipstatusInfoCtx = ctx;
	_onMuxCipstatusInfo = onMuxCipstatusInfo;
}

void SimcomResponseParser::OnGsmModuleEvent(void * ctx, OnGsmModuleEventHandler onGsmModuleEvent)
{
	_onGsmModuleEventCtx = ctx;
	_onGsmModuleEvent = onGsmModuleEvent;
}
