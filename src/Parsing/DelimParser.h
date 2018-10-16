#ifndef __DELIMPARSER_H__
#define __DELIMPARSER_H__

#include <inttypes.h>
#include <string.h>
#include <FixedString.h>
#include <WString.h>

enum ParserStates { INITIAL, INSIDE, START_Q, INSIDE_QUOTE, END_Q, DELIM, ERR, END };

class DelimParser
{
	public:
	FixedString150 _line;
	uint8_t n;
	uint8_t parserState;
	uint8_t tokStart;
	
	bool BeginParsing(FixedString150 &line, const __FlashStringHelper* commandStart);
	void Init(FixedString150 &line, uint8_t n);
	int state_transition(char c, uint8_t state);
	bool NextToken();
	void Skip(int tokenCount);
	bool NextString(FixedStringBase& targetString);
	bool NextNum(uint16_t &dst, int base = 10);

	int hexDigitToInt(char c);

}; //DelimParser

#endif //__DELIMPARSER_H__
