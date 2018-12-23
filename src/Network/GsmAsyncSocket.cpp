#include "GsmAsyncSocket.h"
#include "GsmModule.h"

void GsmAsyncSocket::OnSocketEvent(void *ctx, SocketEventHandler socketEventHandler)
{
	_onSocketEvent = socketEventHandler;
	_onSocketDataReceivedCtx = ctx;
}

void GsmAsyncSocket::OnDataRecieved(void * ctx, SocketDataReceivedHandler onSocketDataReceived)
{
	_onSocketDataReceived = onSocketDataReceived;
	_onSocketDataReceivedCtx = ctx;
}

GsmAsyncSocket::GsmAsyncSocket(SimcomAtCommands& gsm, uint8_t mux, ProtocolType protocol):
_gsm(gsm),
_mux(mux),
_isNetworkAvailable(false),
_protocol(protocol),
_state(SocketStateType::Closed),
_onSocketEventCtx(nullptr),
_onSocketEvent(nullptr),
_onSocketDataReceivedCtx(nullptr),
_onSocketDataReceived(nullptr),
_receivedBytes(0),
_sentBytes(0)
{
}

bool GsmAsyncSocket::IsNetworkAvailable()
{	
	return _isNetworkAvailable;
}

bool GsmAsyncSocket::IsClosed()
{
	return _state == SocketStateType::Closed;
}
bool GsmAsyncSocket::IsConnected()
{
	return _state == SocketStateType::Connected;
}
bool GsmAsyncSocket::BeginConnect(const char* host, uint16_t port)
{
	ChangeState(SocketStateType::Connecting);
	auto connectResult = _gsm.BeginConnect(_protocol, _mux, host, port);
	if (connectResult != AtResultType::Success)
	{
		ChangeState(SocketStateType::Closed);
		return false;
	}
	return true;
}

int16_t GsmAsyncSocket::Send(FixedStringBase & data)
{
	uint16_t sentBytes = 0;
	auto sendResult = _gsm.Send(_mux, data, sentBytes);
	if (sendResult == AtResultType::Success)
	{
		_sentBytes += sentBytes;
		return sentBytes;
	}
	return -1;
}

int16_t GsmAsyncSocket::Send(const char * data, uint16_t length)
{
	FixedString100 dataStr;
	dataStr.append(data, length);
	return Send(dataStr);
}

void GsmAsyncSocket::ChangeState(SocketStateType newState)
{
	_state = newState;
	if (newState == SocketStateType::Closed)
	{
		_receivedBytes = 0;
		_sentBytes = 0;
	}
}

void GsmAsyncSocket::SetIsNetworkAvailable(bool isNetworkAvailable)
{
	_isNetworkAvailable = isNetworkAvailable;

	if (!_isNetworkAvailable)
	{
		if (_state == SocketStateType::Connected || _state == SocketStateType::Connecting)
		{
			RaiseEvent(SocketEventType::Disconnected);
		}
	}
}

void GsmAsyncSocket::RaiseEvent(SocketEventType eventType)
{
	switch (eventType)
	{
	case SocketEventType::ConnectFailed:
		ChangeState(SocketStateType::Closed);
		break;
	case SocketEventType::ConnectSuccess:
		ChangeState(SocketStateType::Connected);
		break;
	case SocketEventType::Disconnecting:
		ChangeState(SocketStateType::Closing);
		break;
	case SocketEventType::Disconnected:
		ChangeState(SocketStateType::Closed);
		break;
	default:
		break;
	}
	if (_onSocketEvent != nullptr)
	{
		_onSocketEvent(this, eventType);
	}
}

bool GsmAsyncSocket::ReadIncomingData()
{
	FixedString100 dataBuffer;
	uint16_t leftData;
	auto gsmReadResult = _gsm.Read(_mux, dataBuffer, leftData);
	if (gsmReadResult != AtResultType::Success)
	{
		if (gsmReadResult == AtResultType::Timeout)
		{
			return false;
		}
		RaiseEvent(SocketEventType::Disconnected);
		return true;
	}
	if (dataBuffer.length() == 0)
	{
		return true;
	}
	_receivedBytes += dataBuffer.length();

	if (_onSocketDataReceived != nullptr)
	{
		_onSocketDataReceived(_onSocketDataReceivedCtx, dataBuffer);
	}
	return true;
}
