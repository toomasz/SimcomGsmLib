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

typedef void(*DataReceivedCallback)(uint8_t mux, FixedStringBase& data);

class SimcomResponseParser
{
	enum LineState { PARSER_INITIAL, PARSER_CR, PARSER_LF, PARSER_LINE };
	uint8_t lineParserState;
	AtCommand _currentCommand;
	ParserState _state;
	CircularDataBuffer& _dataBuffer;
	GsmLogger &_logger;
	FixedString150 _response;
	ParserContext& _parserContext;
	FixedString150 _rxDataBuffer;
	DataReceivedCallback _dataReceivedCallback;
	bool IsErrorLine();
	bool IsOkLine();
	bool ParseUnsolicited(FixedStringBase & line);
	ParserState ParseLine();
	int StateTransition(char c);
	bool _garbageOnSerialDetected;
public:
	SimcomResponseParser(CircularDataBuffer& dataBuffer, ParserContext &parserContext, GsmLogger &logger);
	AtResultType GetAtResultType();
	volatile bool commandReady;
	FunctionBase *function;
	void SetCommandType(AtCommand commandType);
	void FeedChar(char c);	
	void OnDataReceived(DataReceivedCallback onDataReceived);
	bool GarbageOnSerialDetected();
	void ResetUartGarbageDetected();
};


#endif /* PARSERSIM900_H_ */