#ifndef _SIMCOM_RESPONSE_PARSER_H
#define _SIMCOM_RESPONSE_PARSER_H

#include <Arduino.h>
#include "GsmLibConstants.h"
#include "ParserContext.h"
#include "DelimParser.h"
#include "SequenceDetector.h"
#include "GsmLogger.h"
#include <FixedString.h>

typedef void(*DataReceivedCallback)(uint8_t mux, FixedStringBase& data);

class SimcomResponseParser
{
	enum LineState { PARSER_INITIAL, PARSER_CR, PARSER_LF, PARSER_LINE };
	uint8_t lineParserState;
	AtCommand _currentCommand;
	ParserState _state;
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
	Stream& _serial;
	SequenceDetector _promptSequenceDetector;
	FixedStringBase& _currentCommandStr;
public:
	SimcomResponseParser(ParserContext &parserContext, GsmLogger &logger,Stream& serial, FixedStringBase &currentCommandStr);
	AtResultType GetAtResultType();
	volatile bool commandReady;
	void SetCommandType(AtCommand commandType, bool expectEcho = true);
	void FeedChar(char c);	
	void OnDataReceived(DataReceivedCallback onDataReceived);
	bool GarbageOnSerialDetected();
	void ResetUartGarbageDetected();
};


#endif /* PARSERSIM900_H_ */