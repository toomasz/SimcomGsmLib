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
	
	void OnMuxEvent(uint8_t mux, FixedStringBase& eventStr);
	void OnCipstatusInfo(ConnectionInfo& connectionInfo);
public:
	SocketManager(SimcomAtCommands &atCommands, GsmLogger& logger);
	bool ReadDataFromSockets();
	void SetIsNetworkAvailable(bool isNetworkAvailable);
	GsmAsyncSocket* CreateSocket(uint8_t mux, ProtocolType protocolType);
};


#endif
