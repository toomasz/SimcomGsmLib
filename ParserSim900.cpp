#include "ParserSim900.h"

ParserSim900::ParserSim900(Stream& debugStream):
okSeqDetector(PSTR("\r\nOK\r\n")), ds(debugStream)
{
	commandType = 0;
	lineParserState = PARSER_INITIAL;
	n=0;
	lastResult = S900_TIMEOUT;
	function = 0;
	memset(responseBuffer,0, ResponseBufferSize);
}

/* processes character read from serial port of gsm module */
void ParserSim900::FeedChar(char c)
{
	if (commandType == AT_SWITH_TO_COMMAND)
	{
		//printf("w\n");
		gsm->WriteDataBuffer(c);
		if (okSeqDetector.NextChar(c))
		{
			commandReady = true;
			lastResult = S900_OK;
			for (uint8_t i = 0; i < okSeqDetector.length; i++)
				gsm->UnwriteDataBuffer();

		}
		return;
	}
	int prevState = lineParserState;

	lineParserState = StateTransition(c);

	if(prevState  == PARSER_INITIAL || prevState == PARSER_LF || prevState == PARSER_LINE)
	{
		
		if (lineParserState == PARSER_LINE)
		{

			if (commandType == AT_SWITH_TO_COMMAND && gsm->commandBeforeRN)
			{
				gsm->WriteDataBuffer(c);
			}
			if (n < ResponseBufferSize - 2)
				responseBuffer[n++] = c;
		}
	}
	if (lineParserState != PARSER_LINE)
		gsm->commandBeforeRN = false;
	// line -> delimiter
	if ((prevState == PARSER_LINE || prevState == PARSER_CR) && (lineParserState == PARSER_LF))
	{
		responseBuffer[n] =0;
		ds.print("LN: "); ds.println((char*)responseBuffer);
		//	pr("\nLine: '%s' ", responseBuffer);
		if (n == 0)
			return;
			
		crc = crc8(responseBuffer, n);
		int parseResult = ParseLine();

		// if error or or success
		if (parseResult == S900_ERR || parseResult >= 0)
		{
			commandReady = true;
			lastResult = parseResult;
		}
		// if command not parsed yet
		else if (parseResult == S900_NONE)
			lastResult = S900_NONE;
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
int ParserSim900::ParseLine()
{
	if(commandReady)
		return S900_NONE;
		
	if(commandType == AT_CUSTOM_FUNCTION)
		return function->IncomingLine((uint8_t*)responseBuffer, n ,crc);
			
	if(commandType == AT_DEFAULT)
	{
		if (IsErrorLine())
			return S900_ERR;
		if (IsOkLine())
			return S900_OK;
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
			return crc;
		}
	}

	if(commandType == AT_CSQ)
	{
		//+CSQ: 17,0
		if(n > 5 && crc8(responseBuffer, 5) == CRC_CSQ)
		{
			parser.Init((char*)responseBuffer, 6, n);
			bufferedResult = S900_ERR;
			if (parser.NextNum(ctx->signalStrength) && parser.NextNum(ctx->signalErrorRate))
				bufferedResult = S900_OK;
			return S900_NONE;
		}
		if (lastResult == S900_NONE)
		{
			if (IsErrorLine())
				return S900_ERR;
			return bufferedResult;
		}
	}
	if(commandType == AT_SWITCH_TO_DATA)
	{
		if(crc == CRC_CONNECT)		
			return S900_OK;
		
		if(crc == CRC_NO_CARRIER)		
			return S900_ERR;
		
		if(crc == CRC_CLOSED)		
			return S900_ERR;
	}
	if(commandType == AT_SWITH_TO_COMMAND)
	{
		if(crc == CRC_OK)
			return S900_OK;
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
						
				return S900_OK;
			}
		}
		if (IsErrorLine())
			return S900_ERR;
	}
	if(commandType == AT_CIPSTART)
	{
		if(crc == CRC_CONNECT)		
			return S900_OK;
		if(crc == CRC_CONNECT_FAIL || crc == CRC_PDP_DEACT)
			return S900_ERR;
	}
	if(commandType == AT_CIPSHUT)
	{
		if(strcmp_P((char*)responseBuffer, PSTR("SHUT OK")) == 0)
			return S900_OK;
	}
	if(commandType == AT_CIPCLOSE)
	{
		if(strcmp_P((char*)responseBuffer, PSTR("CLOSE OK")) == 0)
		return S900_OK;
	}
	if(commandType == AT_COPS)
	{
		//+COPS: 0,0,"PLAY"
		if(n>7&& crc8(responseBuffer, 6) == CRC_COPS)
		{				
			parser.Init((char*)responseBuffer, 7, n);
			uint16_t tmp;
			if (!parser.NextNum(tmp))
				bufferedResult = S900_ERR;
			else if (!parser.NextNum(tmp))
				bufferedResult = S900_ERR;
			else if (!parser.NextString(ctx->operatorName, 20))
				bufferedResult = S900_ERR;
			else
				bufferedResult = S900_OK;
			return S900_NONE;
		}
		if (lastResult == S900_NONE)
		{
			if (IsErrorLine())
				return S900_ERR;
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
				return S900_NONE;
			}
		}
		if (lastResult == S900_NONE)
		{
			if (IsOkLine())
				return S900_OK;
			if (IsErrorLine())
				return S900_ERR;
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
					bufferedResult = tmp;
					uint16_t lac, cellId;
					if (parser.NextNum(lac, 16) && parser.NextNum(cellId, 16))
					{
						ctx->lac = lac;
						ctx->cellId = cellId;
					}
					return S900_NONE;
				}
			}
			
		}
		if (lastResult == S900_NONE)
		{
			if (IsOkLine())
			{
				if (bufferedResult < 0 || bufferedResult > 5)
					return S900_ERR;
				return bufferedResult;
			}
			if (IsErrorLine())
				return S900_ERR;

		}
	}
	return S900_TIMEOUT;
}

void ParserSim900::SetCommandType(FunctionBase *command)
{
	this->function = command;
	n=0;
	commandReady = false;
	commandType = AT_CUSTOM_FUNCTION;
	lastResult = S900_TIMEOUT;
	bufferedResult = S900_TIMEOUT;
}

void ParserSim900::SetCommandType(int commandType)
{
	//if(commandType != AT_SWITH_TO_COMMAND && commandType != AT_SWITCH_TO_DATA)
	//	while(gsm->ser->available())
	//		gsm->ser->read();
		
	this->commandType = commandType;
	commandReady = false;	
	n = 0;	
	lastResult = S900_TIMEOUT;
	bufferedResult = S900_TIMEOUT;
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


