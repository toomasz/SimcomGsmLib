/*
 * ParserSim900.h
 *
 * Created: 2014-02-09 22:30:36
 *  Author: Tomasz Œcis³owicz
 */ 
#include <Arduino.h>
#include "Sim900Constants.h"
#include "Sim900Crc.h"
#include "Functions/FunctionBase.h"
#include "Sim900.h"
#include "Sim900Context.h"
#include "DelimParser.h"
#include "SequenceDetector.h"
#ifndef PARSERSIM900_H_
#define PARSERSIM900_H_


#define DEBUG4
class Sim900;
class ParserSim900
{
	enum LineState { PARSER_INITIAL, PARSER_CR, PARSER_LF, PARSER_LINE };
	uint8_t lineParserState;
	uint8_t commandType;
	DelimParser parser;
	uint8_t crc;
	SequenceDetector okSeqDetector;

	public:
	Sim900 *gsm;
	Sim900Context *ctx;
	volatile bool commandReady;
	Stream &ds;
	ParserSim900(Stream& debugStream);
	FunctionBase *function;
	void SetCommandType(FunctionBase *command);
	void SetCommandType(int commandType);

	uint8_t responseBuffer[ResponseBufferSize];
	int n;
	bool IsErrorLine();
	bool IsOkLine();

	int lastResult;
	int bufferedResult; // stores actual result for commands like CREG while parser is waiting for OK line
	//int currNumber =
	// this function uses class scope variables responseBuffer, n
	int ParseLine();
	
	int StateTransition(char c);
	void FeedChar(char c);
	
};




#endif /* PARSERSIM900_H_ */