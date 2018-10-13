#include "ParserSim900.h"

#include "MappingHelpers.h"

ParserSim900::ParserSim900(CircularDataBuffer& dataBuffer, GsmLogger& logger):
okSeqDetector(PSTR("\r\nOK\r\n")), 
_dataBuffer(dataBuffer),
_logger(logger)
{
	commandType = 0;
	lineParserState = PARSER_INITIAL;
	n=0;
	lastResult = ParserState::Timeout;
	function = 0;
	memset(responseBuffer,0, ResponseBufferSize);
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
			if (n < ResponseBufferSize - 2)
			{
				responseBuffer[n++] = c;
			}
		}
	}
	if (lineParserState != PARSER_LINE)
	{
		_dataBuffer.commandBeforeRN = false;
	}
	// line -> delimiter
	if ((prevState == PARSER_LINE || prevState == PARSER_CR) && (lineParserState == PARSER_LF))
	{
		responseBuffer[n] =0;
		//	pr("\nLine: '%s' ", responseBuffer);
		if (n == 0)
			return;

		_logger.Log_P(F("  <= %s"), (char*)responseBuffer);

		crc = crc8(responseBuffer, n);
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
		n=0;
	}

}



/* returns true if current line is error: ERROR, CME ERROR etc*/
bool ParserSim900::IsErrorLine()
{
	if(crc == CRC_NO_CARRIER)
		return true;
	if(crc == CRC_PDP_DEACT)
		return true;
		
	if (crc == CRC_ERROR)
		return true;

	if (n > 11)
	{
		if (crc8(responseBuffer, 11) == CRC_CME_ERROR)
			return true;
	}
	return false;
}
/* returns true if current line is OK*/
bool ParserSim900::IsOkLine()
{
	if (crc == CRC_OK)
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
		return function->IncomingLine((uint8_t*)responseBuffer, n, crc);
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
		if(crc == CRC_STATE_IP_INITIAL ||
		crc == CRC_STATE_IP_START ||
		crc == CRC_STATE_IP_CONFIG ||
		crc == CRC_STATE_IP_GPRSACT ||
		crc == CRC_STATE_IP_STATUS ||
		crc == CRC_STATE_TCP_CONNECTING ||
		crc == CRC_STATE_TCP_CLOSED ||
		crc == CRC_STATE_PDP_DEACT ||
		crc == CRC_STATE_CONNECT_OK)
		{
		//	return crc;
		}
	}

	if(commandType == AT_CSQ)
	{
		//+CSQ: 17,0
		if(n > 5 && crc8(responseBuffer, 5) == CRC_CSQ)
		{
			parser.Init((char*)responseBuffer, 6, n);
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
		auto crc = crc8(responseBuffer, 5);
		if (n > 5 && crc == CRC_CBC)
		{
			parser.Init((char*)responseBuffer, 6, n);
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
		if (crc == CRC_CONNECT)
		{
			return ParserState::Success;
		}
		
		if (crc == CRC_NO_CARRIER)
		{
			return ParserState::Error;
		}
		
		if(crc == CRC_CLOSED)		
		{
			return ParserState::Error;
		}
	}
	if(commandType == AT_SWITH_TO_COMMAND)
	{
		if(crc == CRC_OK)
		{
			return ParserState::Success;
		}
	}
	if(commandType == AT_CIFSR)
	{
		if(n>=7)
		{	
			byte dotCount = 0;
			bool hasNonDigits = false;
					
			for(int i=0; i < n; i++)
			{
				if(responseBuffer[i] == '.')
				dotCount++;
				else if(responseBuffer[i] < '0' || responseBuffer[i]>'9')
				hasNonDigits = true;
			}
			if(dotCount == 3 && hasNonDigits == false)
			{
				memcpy(ctx->ipAddress, responseBuffer, n);
				ctx->ipAddress[n] = 0;
						
				return ParserState::Success;
			}
		}
		if (IsErrorLine())
			return ParserState::Error;
	}

	if (commandType == AT_CLCC)
	{		
		if (strstr_P((char*)responseBuffer, (char*)F("+CLCC:")) == (char*)responseBuffer)
		{
			parser.Init((char*)responseBuffer, 6, n);
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
		if(crc == CRC_CONNECT)		
			return ParserState::Success;
		if(crc == CRC_CONNECT_FAIL || crc == CRC_PDP_DEACT)
			return ParserState::Error;
	}
	if(commandType == AT_CIPSHUT)
	{
		if(strcmp_P((char*)responseBuffer, PSTR("SHUT OK")) == 0)
			return ParserState::Success;
	}
	if(commandType == AT_CIPCLOSE)
	{
		if(strcmp_P((char*)responseBuffer, PSTR("CLOSE OK")) == 0)
		return ParserState::Success;
	}
	if(commandType == AT_COPS)
	{
		//+COPS: 0,0,"PLAY"
		if(n>7&& crc8(responseBuffer, 6) == CRC_COPS)
		{				
			parser.Init((char*)responseBuffer, 7, n);
			uint16_t operatorNameFormat;
			if (!parser.NextNum(operatorNameFormat))
			{
				bufferedResult = ParserState::Error;
			}
			else if (!parser.NextNum(operatorNameFormat))
			{
				bufferedResult = ParserState::Error;
			}
			else if (!parser.NextString(ctx->operatorName, 20))
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
		if(n >= 10)
		{
			bool imeiOk = true;
					
			for(int i=0; i < n; i++)
			if(responseBuffer[i] < '0' || responseBuffer[i] > '9')
			imeiOk = false;
					
			if(imeiOk)
			{
				strncpy(ctx->imei, (char*)responseBuffer, n);
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
		if (n > 10)
		{
			if (strcmp_P((char*)responseBuffer, PSTR("+CUSD: ")) == 0)
				return ParserState::Success;
			parser.Init((char*)responseBuffer, 7, n);
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
		if(n>=10 && crc8(responseBuffer,6) == CRC_CREG) // checks if responseBuffer starts with '+CREG:'
		{
			parser.Init((char*)responseBuffer, 7, n);
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
	n=0;
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
	n = 0;	
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