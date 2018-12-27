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

GsmAsyncSocket::GsmAsyncSocket(SimcomAtCommands& gsm, uint8_t mux, ProtocolType protocol, GsmLogger& logger):
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
	_sentBytes(0),
	_logger(logger)
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
	RaiseEvent(SocketEventType::ConnectBegin);
	auto connectResult = _gsm.BeginConnect(_protocol, _mux, host, port);
	if (connectResult != AtResultType::Success)
	{
		RaiseEvent(SocketEventType::ConnectFailed);
		return false;
	}
	return true;
}

bool GsmAsyncSocket::Close()
{
	RaiseEvent(SocketEventType::Disconnecting);
	const auto result = _gsm.CloseConnection(_mux);
	return result == AtResultType::Success;
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
	RaiseEvent(SocketEventType::Disconnected);
	return -1;
}

int16_t GsmAsyncSocket::Send(const char * data, uint16_t length)
{
	FixedString100 dataStr;
	dataStr.append(data, length);
	return Send(dataStr);
}

bool GsmAsyncSocket::ChangeState(SocketStateType newState)
{
	if (_state == newState)
	{
		return false;
	}
	_state = newState;
	if (newState == SocketStateType::Closed)
	{
		_receivedBytes = 0;
		_sentBytes = 0;
	}
	return true;
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

SocketStateType GsmAsyncSocket::EventToState(SocketEventType eventType)
{
	switch (eventType)
	{
	case SocketEventType::ConnectBegin: return SocketStateType::Connecting;
	case SocketEventType::ConnectFailed: return SocketStateType::Closed;
	case SocketEventType::ConnectSuccess: return SocketStateType::Connected;
	case SocketEventType::Disconnecting: return SocketStateType::Closing;
	case SocketEventType::Disconnected:	return SocketStateType::Closed;
	}
	return  SocketStateType::Closed;
}

void GsmAsyncSocket::RaiseEvent(SocketEventType eventType)
{
	auto newState = EventToState(eventType);
	if (newState == SocketStateType::Closing && _state == SocketStateType::Closed)
	{
		return;
	}
	if (!ChangeState(newState))
	{
		return;
	}
	
	if (_onSocketEvent != nullptr)
	{
		_onSocketEvent(this, eventType);
	}
}

bool GsmAsyncSocket::OnMuxEvent(FixedStringBase & eventStr)
{
	if (eventStr == F("CONNECT OK"))
	{
		RaiseEvent(SocketEventType::ConnectSuccess);
	}
	else if (eventStr == F("CONNECT FAIL"))
	{
		RaiseEvent(SocketEventType::ConnectFailed);
	}
	else if (eventStr == F("CLOSED"))
	{
		RaiseEvent(SocketEventType::Disconnected);
	}
	else if (eventStr == F("CLOSE OK"))
	{
		RaiseEvent(SocketEventType::Disconnected);
		// N, CLOSE OK is returned as response to CIPCLOSE=N
		// return false so line is not treated as URC and passed to code 
		// waiting for cipclose response
		return false;
	}
	else
	{
		_logger.Log(F("Failed to parse socket event: %s"), eventStr.c_str());
		return false;
	}
	return true;
}

void GsmAsyncSocket::OnCipstatusInfo(ConnectionInfo &connectionInfo)
{
	switch (connectionInfo.State)
	{
	case ConnectionState::Connecting:
		RaiseEvent(SocketEventType::ConnectBegin);
		break;
	case ConnectionState::Connected:
		RaiseEvent(SocketEventType::ConnectSuccess);
		break;
	case ConnectionState::RemoteClosing:
	case ConnectionState::Closing:
		RaiseEvent(SocketEventType::Disconnecting);
		break;
	case ConnectionState::Closed:
		RaiseEvent(SocketEventType::Disconnected);
		break;
	}
}

bool GsmAsyncSocket::ReadIncomingData()
{
	if (_state != SocketStateType::Connected)
	{
		return true;
	}
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
