#include "DelimParser.h"
#include <stdlib.h> 

DelimParser::DelimParser(FixedStringBase &line, char separator):
	_line(line),
	_separator(separator)
{
	_position = 0;
	_tokenStart = 0;
	_currentState = LineParserState::Initial;
}

bool DelimParser::StartsWith(const __FlashStringHelper* commandStart)
{
	if (!_line.startsWith(commandStart))
	{
		return false;
	}
	_position = strlen_P((PGM_P)commandStart);
	_tokenStart = _position;

	_currentState = LineParserState::Initial;
	return true;
}
void DelimParser::SetSeparator(char separator)
{
	_separator = separator;
}
LineParserState DelimParser::GetNextState(char c, LineParserState state)
{
	switch (state)
	{
	case LineParserState::Error:
		return LineParserState::Error;
	case LineParserState::End:
		return LineParserState::Error;
	case LineParserState::Delimiter:
		if (c == _separator)
		{
			return LineParserState::Delimiter;
		}
		if (c == '"')
		{
			return LineParserState::StartQuote;
		}
		if (c == ' ')
		{
			return LineParserState::Initial;
		}
		return LineParserState::Expression;
	case LineParserState::Initial:
		if (c == ' ')
		{
			return LineParserState::Initial;
		}
		if (c == _separator)
		{
			return LineParserState::Delimiter;
		}
		if (c == '"')
		{
			return LineParserState::StartQuote;
		}
		return LineParserState::Expression;
	case LineParserState::QuotedExpression:
		if (c == '"')
		{
			return LineParserState::EndQuote;
		}
		return LineParserState::QuotedExpression;
	case LineParserState::Expression:
		if (c == _separator)
		{
			return LineParserState::Delimiter;
		}
		return LineParserState::Expression;
	case LineParserState::StartQuote:
		if (c == '\"')
		{
			return LineParserState::EndQuote;
		}
		return LineParserState::QuotedExpression;
	case LineParserState::EndQuote:
		if (c == _separator || c == ' ')
		{
			return LineParserState::Delimiter;
		}
		return LineParserState::Error;
	}
	return LineParserState::Error;
}

bool DelimParser::NextToken()
{
	while (_position <= _line.length() && (_currentState != LineParserState::Error))
	{
		auto previousState = _currentState;
		if (_position == _line.length())
		{
			_currentState = LineParserState::End;
		}
		else
		{
			auto c = _line.c_str()[_position];
			_currentState = GetNextState(c, _currentState);
		}
		//printf("state %s -> %s\n", StateToStr(previousState), StateToStr(_currentState));

		if (previousState == LineParserState::Delimiter && _currentState == LineParserState::Delimiter)
		{
			_tokenStart = _position;
			_position++;
			return true;
		}

		if (previousState != _currentState)
		{
			if (_currentState == LineParserState::Expression)
			{
				_tokenStart = _position;
			}
			else if (_currentState == LineParserState::StartQuote)
			{
				_tokenStart = _position + 1;
			}
			else if (_currentState == LineParserState::Delimiter && previousState == LineParserState::Initial)
			{
				_tokenStart = _position;
				_position++;
				return true;
			}
			// INSIDE -> "
			else if (previousState == LineParserState::Expression)
			{
				_position++;
				return true;
			}
			else if (_currentState == LineParserState::EndQuote)
			{
				_position++;
				return true;
			}
		}
		_position++;
	}
	return false;
}

FixedString128 DelimParser::CurrentToken()
{
	FixedString128 str;
	auto tokEnd = _position - 1;
	if (_tokenStart <= tokEnd)
	{
		str.append(_line.c_str() + _tokenStart, tokEnd - _tokenStart);
	}
	else
	{
		str = "[invalid]";
	}
	return str;
}

bool DelimParser::Skip(int tokenCount)
{
	while (tokenCount-- > 0)
	{
		if (!NextToken())
		{
			return false;
		}
	}
	return true;
}
bool DelimParser::NextString(FixedStringBase& targetString)
{
	if (!NextToken())
	{
		return false;
	}
	int tokLength = _position - 1 - _tokenStart;
	targetString.clear();
	targetString.append(_line.c_str() + _tokenStart, tokLength);
	return true;
}

bool DelimParser::NextNum(uint8_t& dst, bool allowNull, int base)
{
	uint16_t num;
	if (NextNum(num, allowNull, base))
	{
		dst = num;
		return true;
	}
	return false;
}

bool DelimParser::NextNum(int16_t& dst, bool allowNull, int base)
{
	uint16_t num;
	if (NextNum(num, allowNull, base))
	{
		dst = num;
		return true;
	}
	return false;
}
bool DelimParser::NextNum(uint16_t &dst, bool allowNull, int base)
{
	if (!NextToken())
	{
		return false;
	}

	auto tokenEnd = _position - 1;

	if (allowNull && _tokenStart == tokenEnd)
	{
		dst = 0;
		return true;
	}

	dst = 0;

	int x = 1;
	int i = tokenEnd;
	do
	{
		char c = _line.c_str()[i - 1];
		int digitNum = hexDigitToInt(c);
		if (digitNum == -1)
		{
			return false;
		}

		dst += x * digitNum;
		x *= base;
	} while (--i > _tokenStart);

	return true;
}
bool DelimParser::ParseDouble(const char* str, int length, double &target, char decimalSeparator)
{
	if (length == 0)
	{
		return false;
	}
	bool isNegative = str[0] == '-';
	int startIndex = isNegative ? 1 : 0;

	int separatorIndex = -1;
	for (int i = startIndex; i < length; i++)
	{
		if (str[i] == decimalSeparator)
		{
			separatorIndex = i;
		}
	}

	double number = 0;
	int intPartMultiplier = 1;

	auto realPartEnd = separatorIndex != -1 ? separatorIndex - 1 : length - 1;

	for (int i = realPartEnd; i >= startIndex; i--)
	{
		auto c = str[i];
		if (c < '0' || c > '9')
		{
			return false;
		}
		number += (c - '0')*intPartMultiplier;
		intPartMultiplier *= 10;
	}

	if (separatorIndex != -1)
	{
		double decimalPartMultiplier = 0.1;

		for (int i = separatorIndex + 1; i < length; i++)
		{
			auto c = str[i];
			if (c < '0' || c > '9')
			{
				return false;
			}
			number += (c - '0')*decimalPartMultiplier;
			decimalPartMultiplier *= 0.1;
		}
	}
	if (isNegative)
	{
		number = -number;
	}

	target = number;
	return true;
}
bool DelimParser::NextFloat(float & dst)
{
	FixedString32 floatStr;

	if (!NextToken())
	{
		return false;
	}
	
	int tokLength = _position - 1 - _tokenStart;

	double number = 0;
	if (!ParseDouble(_line.c_str() + _tokenStart, tokLength, number))
	{
		return false;
	}

	dst = number;
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

const __FlashStringHelper* DelimParser::StateToStr(LineParserState state)
{
	switch (state)
	{
	case LineParserState::Initial: return F("INITIAL");
	case LineParserState::Expression: return F("INSIDE");
	case LineParserState::StartQuote: return F("START_Q");
	case LineParserState::QuotedExpression: return F("INSIDE_QUOTE");
	case LineParserState::EndQuote: return F("END_Q");
	case LineParserState::Delimiter: return F("DELIM");
	case LineParserState::Error: return F("ERR");
	case LineParserState::End: return F("END");
	}
	return F("");
}


