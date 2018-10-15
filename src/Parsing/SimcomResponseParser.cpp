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
	if (commandType == AtCommand::SwitchToCommand)
	{
		//printf("w\n");
		_dataBuffer.WriteDataBuffer(c);
		if (okSeqDetector.NextChar(c))
		{
			commandReady = true;
			lastResult = ParserState::Success;
			for (uint8_t i = 0; i < okSeqDetector.length; i++)
			{
				_dataBuffer.UnwriteDataBuffer();
			}
		}
		return;
	}
	int prevState = lineParserState;

	lineParserState = StateTransition(c);

	if(prevState  == PARSER_INITIAL || prevState == PARSER_LF || prevState == PARSER_LINE)
	{
		
		if (lineParserState == PARSER_LINE)
		{

			if (commandType == AtCommand::SwitchToCommand && _dataBuffer.commandBeforeRN)
			{
				_dataBuffer.WriteDataBuffer(c);
			}

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
	if (commandReady)
	{
		return ParserState::None;
	}
		
	if (commandType == AtCommand::CustomFunction)
	{
		return function->IncomingLine(_response);
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
		if (ParseIpStatus(_response.c_str(), *_parserContext.IpState))
		{
			return ParserState::Success;
		}
	}

	if(commandType == AtCommand::Csq)
	{
		//+CSQ: 17,0
		if(_response.startsWith(F("+CSQ:")))
		{
			parser.Init((char*)_response.c_str(), 6, _response.length());
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
		if (_response.startsWith(F("+CBC: ")))
		{
			parser.Init((char*)_response.c_str(), 6, _response.length());
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
	if(commandType == AtCommand::SwitchToData)
	{
		if (_response == F("CONNECT"))
		{
			return ParserState::Success;
		}
		
		if (_response == F("NO CARRIER"))
		{
			return ParserState::Error;
		}
		
		if(_response == F("CLOSED"))
		{
			return ParserState::Error;
		}
	}
	if(commandType == AtCommand::SwitchToCommand)
	{
		if(IsOkLine())
		{
			return ParserState::Success;
		}
	}
	if(commandType == AtCommand::Cifsr)
	{
		if (ParsingHelpers::IsIpAddressValid(_response))
		{
			*_parserContext.IpAddress = _response;
			return ParserState::Success;
		}
		if (IsErrorLine())
		{
			return ParserState::Error;
		}
	}

	if (commandType == AtCommand::Clcc)
	{		
		if (_response.startsWith(F("+CLCC:")))
		{
			parser.Init((char*)_response.c_str(), 6, _response.length());
			uint16_t tmp;
			parser.NextNum(tmp);
			parser.NextNum(tmp);
			parser.NextNum(tmp);
			parser.NextNum(tmp);
			parser.NextNum(tmp);

			FixedString20 number;
			
			if (parser.NextString(number))
			{
				_parserContext.CallInfo->CallerNumber = number;
				_parserContext.CallInfo->HasAtiveCall = true;
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
		if(_response.startsWith(F("+COPS:")))
		{				
			parser.Init((char*)_response.c_str(), 7, _response.length());
			uint16_t operatorNameFormat;
			if (!parser.NextNum(operatorNameFormat))
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
				return ParserState::Success;
			if (IsErrorLine())
				return ParserState::Error;
		}
	}
	if (commandType == AtCommand::Cusd)
	{
		if (_response.startsWith(F("+CUSD:")))
		{
			parser.Init((char*)_response.c_str(), 7, _response.length());
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
	if(commandType == AtCommand::Creg)
	{
		// example valid line : +CREG: 2,1,"07E6","D68F"
		if(_response.startsWith(F("+CREG:")))
		{
			parser.Init((char*)_response.c_str(), 7, _response.length());
			uint16_t tmp = 0;
			if (parser.NextNum(tmp))
			{
				if (parser.NextNum(tmp))
				{
					_lastGsmResult = CregToNetworkStatus(tmp);
					bufferedResult = ParserState::Success;
					uint16_t lac, cellId;
					if (parser.NextNum(lac, 16) && parser.NextNum(cellId, 16))
					{
						// todo - use lac and cellId
					}
					return ParserState::None;
				}
			}
			
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
	return ParserState::Timeout;
}

void SimcomResponseParser::SetCommandType(FunctionBase *command)
{
	this->function = command;
	commandReady = false;
	commandType = AtCommand::CustomFunction;
	lastResult = ParserState::Timeout;
	bufferedResult = ParserState::Timeout;
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