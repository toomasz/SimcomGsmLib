#include "DelimParser.h"

bool DelimParser::BeginParsing(FixedString150 &line, const __FlashStringHelper* commandStart)
{
	if (!line.startsWith(commandStart))
	{
		return false;
	}
	_line = line;
	this->tokStart = 0;
	this->n = strlen_P((PGM_P)commandStart);
	parserState = INITIAL;
	return true;
}

int DelimParser::state_transition(char c, uint8_t state)
{
	switch (state)
	{
		case DELIM:
			if (c == ' ')
				return DELIM;
			if (c == ',')
				return ERR;
		case INITIAL:
			if (c != '"')
				return INSIDE;
			return START_Q;

		case INSIDE_QUOTE:
			if (c == '"')
				return END_Q;
			return INSIDE_QUOTE;
		case INSIDE:
			if (c == ',' || c == ' ')
				return DELIM;
			return INSIDE;
		case START_Q:
			return INSIDE_QUOTE;
		case END_Q:
			if (c == ',' || c == ' ')
				return DELIM;
			return ERR;
	}
	return ERR;
}

bool DelimParser::NextToken()
{
	while (n <= _line.length() && (parserState != ERR))
	{
		int prevState = parserState;
		parserState = (n == _line.length()) ? END : state_transition(_line.c_str()[n], parserState);

		if (prevState != parserState)
		{
			if (parserState == INSIDE || parserState == INSIDE_QUOTE)
			{
				tokStart = n;
			}
			// INSIDE -> *
			else if (prevState == INSIDE || prevState == INSIDE_QUOTE)
			{				
				n++;
				return true;
			}
		}
		n++;
	}
	return false;
}

void DelimParser::Skip(int tokenCount)
{
	while (tokenCount-- > 0)
	{
		NextToken();
	}
}
bool DelimParser::NextString(FixedStringBase& targetString)
{
	if (!NextToken())
	{
		return false;
	}
	int tokLength = n - 1 - tokStart;
	targetString.clear();
	targetString.append(_line.c_str() + tokStart, tokLength);
	return true;
}

bool DelimParser::NextNum(uint16_t &dst, int base /*= 10*/)
{
	if (!NextToken())
	{
		return false;
	}
	dst = 0;

	int x = 1;
	int i = n - 1;
	do
	{
		char c = _line.c_str()[i - 1];
		int digitNum = hexDigitToInt(c);
		if (digitNum == -1)
		{
			return false;
		}

		dst += x*digitNum;
		x *= base;
	}
	while (--i > tokStart);

	return true;
}

int DelimParser::hexDigitToInt(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return -1;
}
