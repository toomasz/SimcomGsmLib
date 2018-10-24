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
	lastResult = ParserState::Timeout;
	function = 0;
}

AtResultType SimcomResponseParser::GetAtResultType()
{
	switch (lastResult)
	{
	case ParserState::Success:
		return AtResultType::Success;
	case ParserState::Error:
		return AtResultType::Error;
	case ParserState::Timeout:
	case ParserState::None:
		return AtResultType::Timeout;
	default:
		return AtResultType::Error;
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
	if (lineParserState != PARSER_LINE)
	{
		_dataBuffer.commandBeforeRN = false;
	}
	// line -> delimiter
	if ((prevState == PARSER_LINE || prevState == PARSER_CR) && (lineParserState == PARSER_LF))
	{
		if (_response.length() == 0)
			return;

		_logger.Log_P(F("  <= %s"), (char*)_response.c_str());

		ParserState parseResult = ParseLine();

		// if error or or success
		if (parseResult == ParserState::Success || parseResult == ParserState::Error)
		{
			commandReady = true;
			lastResult = parseResult;
		}
		// if command not parsed yet
		else if (parseResult == ParserState::None)
		{
			lastResult = ParserState::None;
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

	if (commandReady)
	{
		return ParserState::None;
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
						return ParserState::Error;
					}
					if (ipAddressStr.length() > 0 && !ParsingHelpers::ParseIpAddress(ipAddressStr, connInfo->RemoteAddress))
					{
						return ParserState::Error;
					}

					connInfo->Port = port;
					connInfo->State = connectionState;
					return ParserState::None;
				}
			}
			return ParserState::Error;
		}
		if (lastResult == ParserState::None)
		{
			if (IsOkLine())
			{
				return ParserState::Success;
			}
			if (IsErrorLine())
			{
				return ParserState::Error;
			}
		}
	}

	if(commandType == AtCommand::Csq)
	{
		//+CSQ: 17,0
		if(parser.StartsWith(F("+CSQ: ")))
		{
			bufferedResult = ParserState::Error;
			uint16_t signalQuality;
			uint16_t signalStrength;

			if (parser.NextNum(signalQuality) && parser.NextNum(signalStrength))
			{
				*_parserContext.CsqSignalQuality = signalQuality;
				bufferedResult = ParserState::Success;
			}
			return ParserState::None;
		}
		if (lastResult == ParserState::None)
		{
			if (IsErrorLine())
			{
				return ParserState::Error;
			}
			return bufferedResult;
		}
	}

	if (commandType == AtCommand::Cbc)
	{
		if (parser.StartsWith(F("+CBC: ")))
		{
			bufferedResult = ParserState::Error;

			uint16_t batteryPercent;
			uint16_t mVbatteryVoltage;

			parser.Skip(1);

			if (parser.NextNum(batteryPercent)
				&& parser.NextNum(mVbatteryVoltage))
			{
				_parserContext.BatteryInfo->Voltage = mVbatteryVoltage / 1000.0;
				_parserContext.BatteryInfo->Percent = batteryPercent;
				bufferedResult = ParserState::Success;
			}
			return ParserState::None;
		}
		if (lastResult == ParserState::None)
		{
			if (IsOkLine())
			{
				return bufferedResult;
			}
			if (IsErrorLine())
			{
				return ParserState::Error;
			}
		}
	}

	if(commandType == AtCommand::Cifsr)
	{
		GsmIp ip;
		if (ParsingHelpers::ParseIpAddress(_response, ip))
		{
			*_parserContext.IpAddress = ip;
			return ParserState::None;
		}
		if (lastResult == ParserState::None)
		{
			if (IsOkLine())
			{
				return ParserState::Success;
			}
			if (IsErrorLine())
			{
				return ParserState::Error;
			}
		}
	}

	if (commandType == AtCommand::Clcc)
	{		
		if (parser.StartsWith(F("+CLCC: ")))
		{
			parser.Skip(5);
			FixedString20 number;			
			if (parser.NextString(number))
			{
				_parserContext.CallInfo->CallerNumber = number;
				_parserContext.CallInfo->HasIncomingCall = true;
			}
		}
		if (IsOkLine())
		{
			return ParserState::Success;
		}
		if (IsErrorLine())
		{
			return ParserState::Error;
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
				bufferedResult = ParserState::Error;
			}
			else if (!parser.NextNum(operatorNameFormat))
			{
				bufferedResult = ParserState::Error;
			}
			else if (!parser.NextString(*_parserContext.OperatorName))
			{
				bufferedResult = ParserState::Error;
			}
			else
			{
				_logger.Log_P(F("Op selection mode: %d"), _parserContext.operatorSelectionMode);
				_parserContext.IsOperatorNameReturnedInImsiFormat = operatorNameFormat == 2;
				bufferedResult = ParserState::Success;
			}
			return ParserState::None;
		}
		if (lastResult == ParserState::None)
		{
			if (IsErrorLine())
				return ParserState::Error;
			return bufferedResult;
		}
				
	}
	if(commandType == AtCommand::Gsn)
	{
		auto imeiValid = ParsingHelpers::IsImeiValid(_response);
		if (imeiValid)
		{
			*_parserContext.Imei = _response;
			return ParserState::None;
		}		
		if (lastResult == ParserState::None)
		{
			if (IsOkLine())
			{
				return ParserState::Success;
			}
			if (IsErrorLine())
			{
				return ParserState::Error;
			}
		}
	}
	if (commandType == AtCommand::Cusd)
	{
		if (parser.StartsWith(F("+CUSD: ")))
		{
			uint16_t tmp = 0;
			if (parser.NextNum(tmp))
			{
				if (parser.NextString(*_parserContext.UssdResponse))
				{
					return ParserState::Success;
				}
			}
			return ParserState::Error;
		}
	}
	if (commandType == AtCommand::Cipmux)
	{
		if (parser.StartsWith(F("+CIPMUX: ")))
		{
			uint16_t isEnabled;
			if (parser.NextNum(isEnabled))
			{
				_parserContext.Cipmux = isEnabled == 1;
				return ParserState::None;
			}
			return ParserState::Error;
		}
		if (lastResult == ParserState::None)
		{
			if (IsOkLine())
			{
				return ParserState::Success;
			}
			if (IsErrorLine())
			{
				return ParserState::Error;
			}
		}
	}
	if(commandType == AtCommand::Creg)
	{
		// example valid line : +CREG: 2,1,"07E6","D68F"
		if(parser.StartsWith(F("+CREG: ")))
		{
			if (!parser.Skip(1))
			{
				Serial.println("1");
				return ParserState::Error;
			}
			uint8_t cregRegistrationState;
			if (!parser.NextNum(cregRegistrationState))
			{
				Serial.println("2");

				return ParserState::Error;
			}

			if (!ParsingHelpers::ParseRegistrationStatus(cregRegistrationState, _parserContext.RegistrationStatus))
			{
				Serial.println("3");

				return ParserState::Error;
			}			
			lastResult = ParserState::None;
		}
		if (lastResult == ParserState::None)
		{
			if (IsOkLine())
			{				
				return ParserState::Success;
			}
			if (IsErrorLine())
			{
				return ParserState::Error;
			}
		}
	}
	return ParserState::Timeout;
}

void SimcomResponseParser::SetCommandType(AtCommand commandType)
{
	//if(commandType != AT_SWITH_TO_COMMAND && commandType != AT_SWITCH_TO_DATA)
	//	while(gsm->ser->available())
	//		gsm->ser->read();
		
	this->commandType = commandType;
	commandReady = false;	
	_response.clear();
	lastResult = ParserState::Timeout;
	bufferedResult = ParserState::Timeout;
}

int SimcomResponseParser::StateTransition(char c)
{
	switch (lineParserState)
	{
		case PARSER_INITIAL:
			if (c == '\r')
				return PARSER_CR;
			else if (c == '\n')
				return PARSER_LF;
			else
				return PARSER_LINE;
		case PARSER_CR:
			if (c == '\r')
				return PARSER_CR;
			else if (c == '\n')
				return PARSER_LF;
			else
				return PARSER_INITIAL;
		case PARSER_LF:
			if (c == '\r')
				return PARSER_CR;
			else if (c == '\n')
				return PARSER_LF;
			else
				return PARSER_LINE;
		case PARSER_LINE:
			if (c == '\r')
				return PARSER_CR;
			else if (c == '\n')
				return PARSER_LF;
			else
				return PARSER_LINE;

	}

}