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