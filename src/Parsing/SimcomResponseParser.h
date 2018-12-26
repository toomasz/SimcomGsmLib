#ifndef _SIMCOM_RESPONSE_PARSER_H
#define _SIMCOM_RESPONSE_PARSER_H

#include <Arduino.h>
#include "GsmLibConstants.h"
#include "ParserContext.h"
#include "DelimParser.h"
#include "SequenceDetector.h"
#include "GsmLogger.h"
#include <FixedString.h>

typedef void(*MuxEventHandler)(void* ctx, uint8_t mux, FixedStringBase& eventStr);
typedef void(*MuxCipstatusInfoHandler)(void* ctx, ConnectionInfo& info);
class SimcomResponseParser
{
	enum LineState { PARSER_INITIAL, PARSER_CR, PARSER_LF, PARSER_LINE };
	uint8_t lineParserState;
	ParserState _state;
	GsmLogger &_logger;
	FixedString150 _response;
	ParserContext& _parserContext;
	bool IsErrorLine();
	bool IsOkLine();
	bool ParseUnsolicited(FixedStringBase & line);
	ParserState ParseLine();
	int StateTransition(char c);
	bool _garbageOnSerialDetected;
	Stream& _serial;
	SequenceDetector _promptSequenceDetector;
	AtCommand _currentCommand;
	FixedStringBase& _currentCommandStr;

	MuxEventHandler _onMuxEvent;
	void* _onMuxEventCtx;

	MuxCipstatusInfoHandler _onMuxCipstatusInfo;
	void* _onMuxCipstatusInfoCtx;
public:
	SimcomResponseParser(ParserContext &parserContext, GsmLogger &logger,Stream& serial, FixedStringBase &currentCommandStr);
	AtResultType GetAtResultType();
	volatile bool commandReady;
	void SetCommandType(AtCommand commandType, bool expectEcho = true);
	void FeedChar(char c);	
	bool GarbageOnSerialDetected();
	void ResetUartGarbageDetected();
	void OnMuxEvent(void* ctx, MuxEventHandler onMuxEvent);
	void OnMuxCipstatusInfo(void* ctx, MuxCipstatusInfoHandler onMuxCipstatusInfo);
	bool IsGarbageDetectionActive;
};


#endif