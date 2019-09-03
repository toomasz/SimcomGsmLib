#include "SocketManager.h"


SocketManager::SocketManager(SimcomAtCommands &atCommands, GsmLogger& logger) :
	_logger(logger),
	_atCommands(atCommands),
	_sockets{ nullptr },
	_isNetworkAvailable(false)
{
	atCommands.OnMuxEvent(this, [](void* ctx, uint8_t mux, FixedStringBase& eventStr)
	{
		return reinterpret_cast<SocketManager*>(ctx)->OnMuxEvent(mux, eventStr);
	});
	atCommands.OnCipstatusInfo(this, [](void* ctx, ConnectionInfo& connectionInfo)
	{
		reinterpret_cast<SocketManager*>(ctx)->OnCipstatusInfo(connectionInfo);
	});
}

bool SocketManager::OnMuxEvent(uint8_t mux, FixedStringBase& eventStr)
{
	auto socket = _sockets[mux];
	if (socket == nullptr)
	{
		_logger.Log(F("Received socket event but socket was null"));
		return false;
	}
	return socket->OnMuxEvent(eventStr);
}
void SocketManager::OnCipstatusInfo(ConnectionInfo& connectionInfo)
{
	auto mux = connectionInfo.Mux;
	if (mux >= SocketCount)
	{
		return;
	}
	auto socket = _sockets[mux];
	if (socket == nullptr)
	{
		return;
	}
	socket->OnCipstatusInfo(connectionInfo);
}

bool SocketManager::SendDataFromSockets()
{
	for (int i = 0; i < SocketCount; i++)
	{
		auto socket = _sockets[i];
		if (socket == nullptr)
		{
			continue;
		}

		if (!socket->SendPendingData())
		{
			return false;
		}
	}
	return true;
}

bool SocketManager::AnyConnectAtTimeouted()
{
	bool anySocketHasAtConnectTimeout = false;
	for (int i = 0; i < SocketCount; i++)
	{
		auto socket = _sockets[i];
		if (socket == nullptr)
		{
			continue;
		}

		if (socket->GetAndResetHasConnectTimeout())
		{
			anySocketHasAtConnectTimeout = true;
		}
	}
	return anySocketHasAtConnectTimeout;
}

bool SocketManager::ReadDataFromSockets()
{
	for (int i = 0; i < SocketCount; i++)
	{
		auto socket = _sockets[i];
		if (socket == nullptr)
		{
			continue;
		}

		if (!socket->ReadIncomingData())
		{
			return false;
		}
	}
	return true;
}
void SocketManager::SetIsNetworkAvailable(bool isNetworkAvailable)
{
	_isNetworkAvailable = isNetworkAvailable;
	for (int i = 0; i < SocketCount; i++)
	{
		auto socket = _sockets[i];
		if (socket != nullptr)
		{
			socket->SetIsNetworkAvailable(isNetworkAvailable);
		}
	}
}

GsmAsyncSocket* SocketManager::CreateSocket(uint8_t mux, ProtocolType protocolType)
{
	if (mux >= SocketCount)
	{
		_logger.Log(F("Invalid socket number: %d"), mux);
		return nullptr;
	}
	if (_sockets[mux] != nullptr)
	{
		_logger.Log(F("Socket %d is already created"), mux);
		return nullptr;
	}
	auto socket = new GsmAsyncSocket(_atCommands, mux, protocolType, _logger);
	_sockets[mux] = socket;
	_logger.Log(F("Socket %d created"), mux);
	return socket;
}