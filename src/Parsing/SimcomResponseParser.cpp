#include "SimcomResponseParser.h"

#include "GsmLibHelpers.h"
#include "ParsingHelpers.h"

SimcomResponseParser::SimcomResponseParser(CircularDataBuffer& dataBuffer, ParserContext& parserContext, GsmLogger& logger):
_dataBuffer(dataBuffer),
_parserContext(parserContext),
_logger(logger),
_dataReceivedCallback(nullptr),
_garbageOnSerialDetected(false)
{
	_currentCommand = AtCommand::Generic;
	lineParserState = PARSER_INITIAL;
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
	if (_parserContext.CiprxGetLeftBytesToRead > 0)
	{
		_parserContext.CipRxGetBuffer->append(c);
		_parserContext.CiprxGetLeftBytesToRead--;
		return;
	}	
	int prevState = lineParserState;

	lineParserState = StateTransition(c);

	if(prevState  == PARSER_INITIAL || prevState == PARSER_LF || prevState == PARSER_LINE)
	{		
		if (lineParserState == PARSER_LINE)
		{
			_response.append(c);
		}
	}
	// line -> delimiter
	if ((prevState == PARSER_LINE || prevState == PARSER_CR) && (lineParserState == PARSER_LF))
	{
		if (_response.length() == 0)
		{
			return;
		}

		_logger.LogAt(F("  <= %s"), (char*)_response.c_str());

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
		else if (parseResult == ParserState::PartialSuccess || parseResult == ParserState::PartialError)
		{
			_state = parseResult;
		}
		// if command not parsed yet
		else if (parseResult == ParserState::None)
		{
			if (ParsingHelpers::CheckIfLineContainsGarbage(_response))
			{
				_garbageOnSerialDetected = true;

				FixedString200 garbageStr;
				for (int i = 0; i < _response.length(); i++)
				{
					garbageStr.appendFormat(" %2x", _response[i]);
				}
				_logger.Log(F("Garbage detected: '"), garbageStr.c_str(), garbageStr.length());

			}
			else
			{
				_logger.Log(F("Unknown response(%d b): %s\n"), _response.length(), _response.c_str());
			}
			// do nothing, do not change _state to none
		}
		
		_response.clear();
	}

}

void SimcomResponseParser::OnDataReceived(DataReceivedCallback onDataReceived)
{
	_dataReceivedCallback = onDataReceived;
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

bool SimcomResponseParser::ParseUnsolicited(FixedStringBase& line)
{
	DelimParser parser(line);
	if (parser.StartsWith(F("+RECEIVE,")))
	{
		uint8_t mux;
		uint16_t dataLength;
		if (!parser.NextNum(mux))
		{
			return false;
		}
		parser.SetSeparator(':');
		if (!parser.NextNum(dataLength))
		{
			return false;
		}
		return true;		
	}
	return false;
}

ParserState SimcomResponseParser::ParseLine()
{
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
			uint8_t mux;
			uint8_t bearer;
			FixedString20 protocolStr;
			FixedString20 ipAddressStr;
			uint16_t port;
			FixedString20 connectionStateStr;
			if (parser.NextNum(mux) &&
				parser.NextNum(bearer, true) &&
				parser.NextString(protocolStr) &&
				parser.NextString(ipAddressStr) &&
				parser.NextNum(port, true) &&
				parser.NextString(connectionStateStr))
			{
				ProtocolType protocolType;
				ConnectionState connectionState;

				if(ParsingHelpers::ParseConnectionState(connectionStateStr, connectionState))
				{
					auto connInfo = _parserContext.CurrentConnectionInfo;

					connInfo->Mux = mux;
					connInfo->Bearer = bearer;

					if (protocolStr.length() > 0 && !ParsingHelpers::ParseProtocolType(protocolStr, connInfo->Protocol))
					{
						return ParserState::PartialError;
					}
					if (ipAddressStr.length() > 0 && !ParsingHelpers::ParseIpAddress(ipAddressStr, connInfo->RemoteAddress))
					{
						return ParserState::PartialError;
					}

					connInfo->Port = port;
					connInfo->State = connectionState;
					return ParserState::PartialSuccess;
				}
			}
			return ParserState::PartialError;
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
			FixedString20 number;			
			if (!parser.NextString(number))
			{
				return ParserState::PartialError;
			}
			_parserContext.CallInfo->CallerNumber = number;
			_parserContext.CallInfo->HasIncomingCall = true;			
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
		if (_response.equals(F("CLOSE OK")))
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
		if(parser.StartsWith(F("DATA ACCEPT:")))
		{
			return ParserState::Success;
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

void SimcomResponseParser::SetCommandType(AtCommand command)
{		
	_currentCommand = command;
	commandReady = false;	
	_state = ParserState::Timeout;
}

int SimcomResponseParser::StateTransition(char c)
{
	switch (lineParserState)
	{
		case PARSER_INITIAL:
			if (c == '\r')
			{
				return PARSER_CR;
			}
			else if (c == '\n')
			{
				return PARSER_LF;
			}
			else
			{
				return PARSER_LINE;
			}
		case PARSER_CR:
			if (c == '\r')
			{
				return PARSER_CR;
			}
			else if (c == '\n')
			{
				return PARSER_LF;
			}
			else
			{
				return PARSER_INITIAL;
			}
		case PARSER_LF:
			if (c == '\r')
			{
				return PARSER_CR;
			}
			else if (c == '\n')
			{
				return PARSER_LF;
			}
			else
			{
				return PARSER_LINE;
			}
		case PARSER_LINE:
			if (c == '\r')
			{
				return PARSER_CR;
			}
			else if (c == '\n')
			{
				return PARSER_LF;
			}
			else
			{
				return PARSER_LINE;
			}

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