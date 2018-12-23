#ifndef _GSM_ASYNC_SOCKET_H
#define _GSM_ASYNC_SOCKET_H

#include <inttypes.h>
#include "SimcomGsmTypes.h"
#include "SimcomAtCommands.h"
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
	ConnectFailed,
	ConnectSuccess,
	Disconnecting,
	Disconnected,
};

typedef void(*SocketEventHandler)(void* ctx, SocketEventType eventType);
typedef void(*SocketDataReceivedHandler)(void *ctx, FixedStringBase& data);

class SocketManager;

class GsmAsyncSocket
{
	friend class SocketManager;

	uint8_t _mux;
	bool _isNetworkAvailable;
	ProtocolType _protocol;
	SimcomAtCommands& _gsm;
	void* _onSocketEventCtx;
	SocketEventHandler _onSocketEvent;
	void* _onSocketDataReceivedCtx;
	SocketDataReceivedHandler _onSocketDataReceived;
	SocketStateType _state;
	void ChangeState(SocketStateType newState);
	void SetIsNetworkAvailable(bool isNetworkAvailable);
	void RaiseEvent(SocketEventType eventType);
	bool ReadIncomingData();
	uint64_t _receivedBytes;
	uint64_t _sentBytes;
public:
	GsmAsyncSocket(SimcomAtCommands& gsm, uint8_t mux, ProtocolType protocol);
	SocketStateType GetState()
	{
		return _state;
	}
	void OnSocketEvent(void *ctx, SocketEventHandler socketEventHandler);
	void OnDataRecieved(void *ctx, SocketDataReceivedHandler onSocketDataReceived);
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
	int16_t Send(FixedStringBase& data);
	int16_t Send(const char* data, uint16_t length);
};

#endif
