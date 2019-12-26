#ifndef  _OPERATOR_NAME_HELPER_H
#define _OPERATOR_NAME_HELPER_H


#include "SimcomAtCommands.h"
#include <FixedString.h>

class OperatorNameHelper
{
	static const char* _gsmNetworks[][2];
	static const char *GetRealNetworkName(const char* networkName);
public:
	static AtResultType GetRealOperatorName(SimcomAtCommands& gsm, FixedString32&operatorName);
};


#endif

