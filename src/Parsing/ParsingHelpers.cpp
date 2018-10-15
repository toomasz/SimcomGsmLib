#include "ParsingHelpers.h"


bool ParsingHelpers::IsImeiValid(FixedStringBase &imei)
{
	if (imei.length() < 10)
	{
		return false;
	}
	if (imei.length() > 18)
	{
		return false;
	}
	for (int i = 0; i < imei.length(); i++)
	{
		auto c = imei.c_str()[i];
		if (c < '0' || c > '9')
		{
			return false;
		}
	}
	return true;
}
bool ParsingHelpers::IsIpAddressValid(FixedStringBase &ipAddress)
{
	if (ipAddress.length() < 7)
	{
		return false;
	}
	
	byte dotCount = 0;

	for (int i = 0; i < ipAddress.length(); i++)
	{
		auto c = ipAddress.c_str()[i];
		if (c == '.')
		{
			dotCount++;
		}
		else if (c < '0' || c >'9')
		{
			return false;
		}
	}

	return dotCount == 3;	
}