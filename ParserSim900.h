#ifndef _PARSERSIM900_H_
#define _PARSERSIM900_H_

#include <Arduino.h>
#include "Sim900Constants.h"
#include "Sim900Crc.h"
#include "Functions/FunctionBase.h"
#include "Sim900.h"
#include "Sim900Context.h"
#include "DelimParser.h"
#include "SequenceDetector.h"

class Sim900;

class ParserSim900
{
	enum LineState { PARSER_INITIAL, PARSER_CR, PARSER_LF, PARSER_LINE };
	uint8_t lineParserState;
	uint8_t commandType;
	DelimParser parser;
	uint8_t crc;
	SequenceDetector okSeqDetector;
	ParserState lastResult;
	ParserState bufferedResult; // stores actual result for commands like CREG while parser is waiting for OK line
	GsmLogCallback _onLog;
public:
	void SetLogCallback(GsmLogCallback onLog);
	GsmNetworkStatus _lastGsmResult;

	AtResultType GetAtResultType();

	Sim900 *gsm;
	Sim900Context *ctx;
	volatile bool commandReady;
	ParserSim900();
	FunctionBase *function;
	void SetCommandType(FunctionBase *command);
	void SetCommandType(int commandType);

	uint8_t responseBuffer[ResponseBufferSize];
	int n;
	bool IsErrorLine();
	bool IsOkLine();

	//int currNumber =
	// this function uses class scope variables responseBuffer, n
	ParserState ParseLine();
	
	int StateTransition(char c);
	void Log_P(const __FlashStringHelper * format, ...);
	void FeedChar(char c);
	
};


#endif /* PARSERSIM900_H_ */