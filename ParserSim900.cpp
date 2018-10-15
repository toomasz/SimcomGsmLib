#include "ParserSim900.h"

#include "MappingHelpers.h"

ParserSim900::ParserSim900(CircularDataBuffer& dataBuffer, GsmLogger& logger):
okSeqDetector(PSTR("\r\nOK\r\n")), 
_dataBuffer(dataBuffer),
_logger(logger)
{
	commandType = 0;
	lineParserState = PARSER_INITIAL;
	lastResult = ParserState::Timeout;
	function = 0;
}

AtResultType ParserSim900::GetAtResultType()
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
void ParserSim900::FeedChar(char c)
{
	if (commandType == AT_SWITH_TO_COMMAND)
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

			if (commandType == AT_SWITH_TO_COMMAND && _dataBuffer.commandBeforeRN)
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
bool ParserSim900::IsErrorLine()
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
bool ParserSim900::IsOkLine()
{
	if (_response == F("OK"))
		return true;
	return false;
}
ParserState ParserSim900::ParseLine()
{
	if (commandReady)
	{
		return ParserState::None;
	}
		
	if (commandType == AT_CUSTOM_FUNCTION)
	{
		return function->IncomingLine(_response);
	}
			
	if(commandType == AT_DEFAULT)
	{
		if (IsErrorLine())
			return ParserState::Error;
		if (IsOkLine())
			return ParserState::Success;
	}
			
			
	if(commandType == AT_CIPSTATUS)
	{
		if (ParseIpStatus(_response.c_str(), ctx->_ipState))
		{
			return ParserState::Success;
		}		
	}

	if(commandType == AT_CSQ)
	{
		//+CSQ: 17,0
		if(_response.startsWith(F("+CSQ:")))
		{
			parser.Init((char*)_response.c_str(), 6, _response.length());
			bufferedResult = ParserState::Error;
			if (parser.NextNum(ctx->signalStrength) && parser.NextNum(ctx->signalErrorRate))
				bufferedResult = ParserState::Success;
			return ParserState::None;
		}
		if (lastResult == ParserState::None)
		{
			if (IsErrorLine())
				return ParserState::Error;
			return bufferedResult;
		}
	}

	if (commandType == AT_CBC)
	{
		if (_response.startsWith(F("+CBC: ")))
		{
			parser.Init((char*)_response.c_str(), 6, _response.length());
			bufferedResult = ParserState::Error;

			uint16_t firstNum;
			if (parser.NextNum(firstNum) &&
				parser.NextNum(ctx->batteryPercent)
				&& parser.NextNum(ctx->batteryVoltage))
			{
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
	if(commandType == AT_SWITCH_TO_DATA)
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
	if(commandType == AT_SWITH_TO_COMMAND)
	{
		if(IsOkLine())
		{
			return ParserState::Success;
		}
	}
	if(commandType == AT_CIFSR)
	{
		if(_response.length() > 7)
		{	
			byte dotCount = 0;
			bool hasNonDigits = false;
					
			for(int i=0; i < _response.length(); i++)
			{
				if(_response.c_str()[i] == '.')
				dotCount++;
				else if(_response.c_str()[i] < '0' || _response.c_str()[i]>'9')
				hasNonDigits = true;
			}
			if(dotCount == 3 && hasNonDigits == false)
			{
				memcpy(ctx->ipAddress, _response.c_str(), _response.length());
				ctx->ipAddress[_response.length()] = 0;
						
				return ParserState::Success;
			}
		}
		if (IsErrorLine())
			return ParserState::Error;
	}

	if (commandType == AT_CLCC)
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

			char numberBuffer[20];

			if (parser.NextString(numberBuffer, 20))
			{
				ctx->_callInfo.CallerNumber = numberBuffer;
				ctx->_callInfo.HasAtiveCall = true;
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

	if(commandType == AT_CIPSTART)
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
	if(commandType == AT_CIPSHUT)
	{
		if (_response.equals(F("SHUT OK")))
		{
			return ParserState::Success;
		}
	}
	if(commandType == AT_CIPCLOSE)
	{
		if (_response.equals(F("CLOSE OK")))
		{
			return ParserState::Success;
		}
	}
	if(commandType == AT_COPS)
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
			else if (!parser.NextString(ctx->_operatorName))
			{
				bufferedResult = ParserState::Error;
			}
			else
			{
				ctx->_isOperatorNameReturnedInImsiFormat = operatorNameFormat == 2;
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
	if(commandType == AT_GSN)
	{
		if(_response.length() > 10)
		{
			bool imeiOk = true;
					
			for (int i = 0; i < _response.length(); i++)
			{
				if (_response.c_str()[i] < '0' || _response.c_str()[i] > '9')
				{
					imeiOk = false;
				}
			}
					
			if(imeiOk)
			{
				strncpy(ctx->imei, (char*)_response.c_str(), _response.length());
				return ParserState::None;
			}
		}
		if (lastResult == ParserState::None)
		{
			if (IsOkLine())
				return ParserState::Success;
			if (IsErrorLine())
				return ParserState::Error;
		}
	}
	if (commandType == AT_CUSD)
	{
		if (_response.startsWith(F("+CUSD:")))
		{
			parser.Init((char*)_response.c_str(), 7, _response.length());
			uint16_t tmp = 0;
			if (parser.NextNum(tmp))
			{
				char ussd[200];
				if (parser.NextString(ctx->buffer_ptr, ctx->buffer_size))
				{
					return ParserState::Success;
				}
			}
			return ParserState::Error;
		}
	}
	if(commandType == AT_CREG)
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
						ctx->lac = lac;
						ctx->cellId = cellId;
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

void ParserSim900::SetCommandType(FunctionBase *command)
{
	this->function = command;
	commandReady = false;
	commandType = AT_CUSTOM_FUNCTION;
	lastResult = ParserState::Timeout;
	bufferedResult = ParserState::Timeout;
}

void ParserSim900::SetCommandType(int commandType)
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

int ParserSim900::StateTransition(char c)
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