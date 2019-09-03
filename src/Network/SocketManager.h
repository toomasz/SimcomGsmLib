#ifndef _SOCKET_MANAGER_H
#define _SOCKET_MANAGER_H

#include "GsmAsyncSocket.h"
#include "../SimcomAtCommands.h"
#include "../GsmLogger.h"

const int SocketCount = 6;

class SocketManager
{
	GsmLogger &_logger;
	SimcomAtCommands& _atCommands;
	GsmAsyncSocket* _sockets[SocketCount];
	bool _isNetworkAvailable;
	
	bool OnMuxEvent(uint8_t mux, FixedStringBase& eventStr);
	void OnCipstatusInfo(ConnectionInfo& connectionInfo);
public:
	SocketManager(SimcomAtCommands &atCommands, GsmLogger& logger);

	bool AnyConnectAtTimeouted();
	bool SendDataFromSockets();
	bool ReadDataFromSockets();
	void SetIsNetworkAvailable(bool isNetworkAvailable);
	GsmAsyncSocket* CreateSocket(uint8_t mux, ProtocolType protocolType);
};


#endif
