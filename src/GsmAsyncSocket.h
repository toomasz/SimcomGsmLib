#pragma once

class GsmModule;

class GsmAsyncSocket
{

public:
	GsmAsyncSocket(GsmModule& gsmModule);
	bool IsNetworkAvailable();
	GsmAsyncSocket();
	~GsmAsyncSocket();
};

