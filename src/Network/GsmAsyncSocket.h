#ifndef _GSM_ASYNC_SOCKET_H
#define _GSM_ASYNC_SOCKET_H

#include <inttypes.h>
#include "../SimcomGsmTypes.h"
#include "../SimcomAtCommands.h"
#include "../GsmLogger.h"
#include <FixedString.h>
class GsmModule;

enum class SocketStateType : uint8_t
{
	Closed,
	Connecting,
	Connected,
	Closing
};

enum class SocketEventType : uint8_t
{
	ConnectBegin,
	ConnectFailed,
	ConnectSuccess,
	Disconnecting,
	Disconnected,
};

typedef void(*SocketEventHandler)(void* ctx, SocketEventType eventType);
typedef void(*SocketDataReceivedHandler)(void *ctx, FixedStringBase& data);
typedef void(*OnPollHandler)(void *ctx);

class SocketManager;

class GsmAsyncSocket
{
	friend class SocketManager;

	uint8_t _mux;
	bool _isNetworkAvailable;
	ProtocolType _protocol;
	SimcomAtCommands& _gsm;
	SocketStateType _state;
	uint64_t _receivedBytes;
	uint64_t _sentBytes;
	GsmLogger& _logger;

	void* _onSocketEventCtx;
	SocketEventHandler _onSocketEvent;
	void* _onSocketDataReceivedCtx;
	SocketDataReceivedHandler _onSocketDataReceived;
	void* _onPollCtx;
	OnPollHandler _onPoll;

	SocketStateType EventToState(SocketEventType eventType);
	bool ChangeState(SocketStateType newState);
	void SetIsNetworkAvailable(bool isNetworkAvailable);
	void RaiseEvent(SocketEventType eventType);
	bool OnMuxEvent(FixedStringBase &eventStr);
	void OnCipstatusInfo(ConnectionInfo& connectionInfo);
	bool ReadIncomingData();	
public:
	GsmAsyncSocket(SimcomAtCommands& gsm, uint8_t mux, ProtocolType protocol, GsmLogger& logger);
	SocketStateType GetState()
	{
		return _state;
	}
	void OnSocketEvent(void *ctx, SocketEventHandler socketEventHandler);
	void OnDataRecieved(void *ctx, SocketDataReceivedHandler onSocketDataReceived);
	void OnPoll(void* ctx, OnPollHandler onPollHandler);
	bool IsNetworkAvailable();
	bool IsClosed();
	bool IsConnected();
	bool BeginConnect(const char* host, uint16_t port);
	uint64_t GetSentBytes()
	{
		return _sentBytes;
	}
	uint64_t GetReceivedBytes()
	{
		return _receivedBytes;
	}
	bool Close();
	int16_t Send(FixedStringBase& data);
	int16_t Send(const char* data, uint16_t length);
	int16_t Send(const char* data);
};

#endif
