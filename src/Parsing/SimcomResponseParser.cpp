#include "SimcomResponseParser.h"

#include "MappingHelpers.h"
#include "ParsingHelpers.h"

SimcomResponseParser::SimcomResponseParser(CircularDataBuffer& dataBuffer, ParserContext& parserContext, GsmLogger& logger):
okSeqDetector(PSTR("\r\nOK\r\n")), 
_dataBuffer(dataBuffer),
_parserContext(parserContext),
_logger(logger)
{
	commandType = AtCommand::Generic;
	lineParserState = PARSER_INITIAL;
	_state = ParserState::Timeout;
	function = 0;
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

		_logger.Log_P(F("  <= %s"), (char*)_response.c_str());

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
			// do nothing, do not change _state to none
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
	if (_response == F("+PDP: DEACT"))
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
	if (_response == F("OK"))
		return true;
	return false;
}
ParserState SimcomResponseParser::ParseLine()
{
	DelimParser parser(_response);
			
	if (IsOkLine())
	{
		if (_state == ParserState::PartialSuccess)
		{
			return ParserState::Success;
		}		
	}
	if (IsErrorLine())
	{
		return ParserState::Error;
	}	

	if(commandType == AtCommand::Generic)
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

	if(commandType == AtCommand::Cipstatus)
	{
		if (ParsingHelpers::ParseIpStatus(_response.c_str(), *_parserContext.IpState))
		{
			return ParserState::Success;
		}
	}
	if (commandType == AtCommand::CipstatusSingleConnection)
	{
		if (parser.StartsWith(F("+CIPSTATUS: ")))
		{
			_logger.Log_P(F("Begin parsing sipstatus"));

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
						Serial.printf("Failed to parser proto: %s\n", protocolStr.c_str());
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

	if(commandType == AtCommand::Csq)
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

	if (commandType == AtCommand::Cbc)
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

	if(commandType == AtCommand::Cifsr)
	{
		GsmIp ip;
		if (ParsingHelpers::ParseIpAddress(_response, ip))
		{
			*_parserContext.IpAddress = ip;
			return ParserState::PartialSuccess;
		}
	}

	if (commandType == AtCommand::Clcc)
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
		// CLCC can return no records so its ok
		if (IsOkLine())
		{
			return ParserState::Success;
		}
	}

	if(commandType == AtCommand::Cipstart)
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
	if(commandType == AtCommand::Cipshut)
	{
		if (_response.equals(F("SHUT OK")))
		{
			return ParserState::Success;
		}
	}
	if(commandType == AtCommand::Cipclose)
	{
		if (_response.equals(F("CLOSE OK")))
		{
			return ParserState::Success;
		}
	}
	if(commandType == AtCommand::Cops)
	{
		//+COPS: 0,0,"PLAY"
		if(parser.StartsWith(F("+COPS: ")))
		{				
			uint16_t operatorNameFormat;

			if (!parser.NextNum(_parserContext.operatorSelectionMode))
			{
				return ParserState::PartialError;
			}
			else if (!parser.NextNum(operatorNameFormat))
			{
				return ParserState::PartialError;
			}
			else if (!parser.NextString(*_parserContext.OperatorName))
			{
				return ParserState::PartialError;
			}
			else
			{
				_logger.Log_P(F("Op selection mode: %d"), _parserContext.operatorSelectionMode);
				_parserContext.IsOperatorNameReturnedInImsiFormat = operatorNameFormat == 2;
				return ParserState::PartialSuccess;
			}
		}				
	}
	if(commandType == AtCommand::Gsn)
	{
		auto imeiValid = ParsingHelpers::IsImeiValid(_response);
		if (imeiValid)
		{
			*_parserContext.Imei = _response;
			return ParserState::PartialSuccess;
		}
	}
	if (commandType == AtCommand::Cusd)
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
	if (commandType == AtCommand::Cipmux)
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
	if(commandType == AtCommand::Creg)
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
	return ParserState::None;
}

void SimcomResponseParser::SetCommandType(AtCommand commandType)
{		
	this->commandType = commandType;
	commandReady = false;	
	_response.clear();
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