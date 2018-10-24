#ifndef _PARSERSIM900_H_
#define _PARSERSIM900_H_

#include <Arduino.h>
#include "GsmLibConstants.h"
#include "Functions/FunctionBase.h"
#include "CircularDataBuffer.h"
#include "ParserContext.h"
#include "DelimParser.h"
#include "SequenceDetector.h"
#include "GsmLogger.h"

class SimcomResponseParser
{
	enum LineState { PARSER_INITIAL, PARSER_CR, PARSER_LF, PARSER_LINE };
	uint8_t lineParserState;
	AtCommand commandType;
	SequenceDetector okSeqDetector;
	ParserState lastResult;
	ParserState bufferedResult; // stores actual result for commands like CREG while parser is waiting for OK line
	CircularDataBuffer& _dataBuffer;
	GsmLogger &_logger;
	FixedString150 _response;
	ParserContext& _parserContext;
public:
	SimcomResponseParser(CircularDataBuffer& dataBuffer, ParserContext &parserContext, GsmLogger &logger);
	AtResultType GetAtResultType();
	volatile bool commandReady;
	FunctionBase *function;
	void SetCommandType(AtCommand commandType);


	bool IsErrorLine();
	bool IsOkLine();

	//int currNumber =
	// this function uses class scope variables responseBuffer, n
	ParserState ParseLine();
	
	int StateTransition(char c);
	void FeedChar(char c);
	
};


#endif /* PARSERSIM900_H_ */