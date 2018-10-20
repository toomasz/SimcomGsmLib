#include "OperatorNameHelper.h"

char* OperatorNameHelper::_gsmNetworks[][2] =
{
	{"26001", "Plus"},
	{"26002", "T-Mobile"},
	{"26003", "Orange"},
	{"26006", "Play"},
	{ nullptr, nullptr }
};

AtResultType OperatorNameHelper::GetRealOperatorName(SimcomGsm& gsm, FixedString20&operatorName)
{
	FixedString20 netowrkNameImsi;
	auto result = gsm.GetOperatorName(netowrkNameImsi, true);
	if (result != AtResultType::Success)
	{
		return result;
	}
	auto realName = GetRealNetworkName(netowrkNameImsi.c_str());
	if (realName != nullptr)
	{
		operatorName = realName;
	}
	else
	{
		operatorName = netowrkNameImsi;
	}
	return result;
}

const char* OperatorNameHelper::GetRealNetworkName(const char* networkName)
{
	int i = 0;
	auto gsmNetwork = _gsmNetworks[0];
	while (gsmNetwork[0] != nullptr)
	{
		if (strcmp(networkName, gsmNetwork[0]) == 0)
		{
			return gsmNetwork[1];
		}
		gsmNetwork++;
	}
	return networkName;
}
