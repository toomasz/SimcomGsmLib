#ifndef _SOCKET_MANAGER_H
#define _SOCKET_MANAGER_H

#include "GsmAsyncSocket.h"
#include "../SimcomAtCommands.h"
#include "../GsmLogger.h"

const int SocketCount = 6;

class SocketManager
{
	GsmAsyncSocket* _sockets[SocketCount];
	SimcomAtCommands& _atCommands;
	GsmLogger &_logger;
	bool _isNetworkAvailable;
	
	void OnMuxEvent(uint8_t mux, FixedStringBase& eventStr)
	{
		auto socket = _sockets[mux];
		if (socket == nullptr)
		{
			_logger.Log(F("Received socket event but socket was null"));
			return;
		}
		socket->OnMuxEvent(eventStr);
	}
	void OnCipstatusInfo(ConnectionInfo& connectionInfo)
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

public:
	SocketManager(SimcomAtCommands &atCommands, GsmLogger& logger):
		_atCommands(atCommands),
		_sockets({nullptr}),
		_logger(logger),
		_isNetworkAvailable(false)
	{
		atCommands.OnMuxEvent(this, [](void* ctx, uint8_t mux, FixedStringBase& eventStr)
		{
			reinterpret_cast<SocketManager*>(ctx)->OnMuxEvent(mux, eventStr);
		});
		atCommands.OnCipstatusInfo(this, [](void* ctx, ConnectionInfo& connectionInfo)
		{
			reinterpret_cast<SocketManager*>(ctx)->OnCipstatusInfo(connectionInfo);
		});
	}		

	bool ReadDataFromSockets()
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
	void SetIsNetworkAvailable(bool isNetworkAvailable)
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

	GsmAsyncSocket* CreateSocket(uint8_t mux, ProtocolType protocolType)
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
};


#endif
