#ifndef _SIMCOM_RESPONSE_PARSER_H
#define _SIMCOM_RESPONSE_PARSER_H

#include <Arduino.h>
#include "../GsmLibConstants.h"
#include "ParserContext.h"
#include "DelimParser.h"
#include "SequenceDetector.h"
#include "../GsmLogger.h"
#include <FixedString.h>

typedef bool(*MuxEventHandler)(void* ctx, uint8_t mux, FixedStringBase& eventStr);
typedef void(*MuxCipstatusInfoHandler)(void* ctx, ConnectionInfo& info);
typedef void(*OnGsmModuleEventHandler)(void *ctx, GsmModuleEventType eventType);

enum class LineState 
{
	PARSER_INITIAL,
	PARSER_CR,
	PARSER_LF,
	PARSER_LINE 
};

class SimcomResponseParser
{
	LineState lineParserState;
	ParserState _state;
	GsmLogger &_logger;
	FixedString512 _response;
	ParserContext& _parserContext;	
	bool _garbageOnSerialDetected;
	Stream& _serial;
	SequenceDetector _promptSequenceDetector;
	AtCommand _currentCommand;
	void RaiseGsmModuleEvent(GsmModuleEventType eventType);
	FixedStringBase& _currentCommandStr;

	MuxEventHandler _onMuxEvent;
	void* _onMuxEventCtx;	
	MuxCipstatusInfoHandler _onMuxCipstatusInfo;
	void* _onMuxCipstatusInfoCtx;
	OnGsmModuleEventHandler _onGsmModuleEvent;
	void* _onGsmModuleEventCtx;

	ParserState ParseLine();
	LineState StateTransition(char c);
	bool IsErrorLine();
	bool IsOkLine();
	bool ParseUnsolicited(FixedStringBase & line);
    bool IsCurrentLineAllInHexChars();
public:
	SimcomResponseParser(ParserContext &parserContext, GsmLogger &logger,Stream& serial, FixedStringBase &currentCommandStr);
	AtResultType GetAtResultType();
	void SetCommandType(AtCommand commandType, bool expectEcho = true);
	void FeedChar(char c);	
	bool GarbageOnSerialDetected();
	void ResetUartGarbageDetected();
	void OnMuxEvent(void* ctx, MuxEventHandler onMuxEvent);
	void OnMuxCipstatusInfo(void* ctx, MuxCipstatusInfoHandler onMuxCipstatusInfo);
	void OnGsmModuleEvent(void* ctx, OnGsmModuleEventHandler handler);
	volatile bool commandReady;
	bool IsGarbageDetectionActive;
};


#endif