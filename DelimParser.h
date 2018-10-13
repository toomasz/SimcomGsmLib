/* 
* DelimParser.h
*
* Created: 2014-06-19 15:45:17
* Author: Tomasz Œcis³owicz
*/


#ifndef __DELIMPARSER_H__
#define __DELIMPARSER_H__

#include <inttypes.h>
#include <string.h>
#include <FixedString.h>

enum ParserStates { INITIAL, INSIDE, START_Q, INSIDE_QUOTE, END_Q, DELIM, ERR, END };

class DelimParser
{
	public:
	char *str;
	uint8_t strLength;
	uint8_t n;
	uint8_t parserState;
	uint8_t tokStart;
	
	void Init(char *str, uint8_t n, uint8_t strLength);
	int state_transition(char c, uint8_t state);
	bool NextToken();

	bool NextString(char *d_str, uint8_t length);
	bool NextString(FixedStringBase * d_str);
	bool NextNum(uint16_t &dst, int base = 10);

	int hexDigitToInt(char c);

}; //DelimParser

#endif //__DELIMPARSER_H__
