/* 
* Sim900Strings.h
*
* Created: 2014-06-18 21:52:24
* Author: Tomasz Œcis³owicz
*/


#ifndef __SIM900STRINGS_H__
#define __SIM900STRINGS_H__


class Sim900Strings
{
	public:
	/*static const __FlashStringHelper* ResultToString(int result)
	{
		switch(result)
		{
			case S900_ERR:		return F("S900_ERR");
			case S900_OK:		return F("S900_OK");
			case S900_TIMEOUT:	return F("S900_TIMEOUT");
			case S900_NONE:		return F("S900_NONE");
		}
		return F("UNK!");
	}
	static const __FlashStringHelper * RegStatus2Pstr(int regStatus)
	{
		switch(regStatus)
		{
			case SEARCHING_FOR_NETWORK0:	return F("SEARCH_FOR_NET0");
			case HOME_NETWORK:				return F("HOME_NETWORK");
			case SEARCHING_FOR_NETWORK:		return F("SEARCH_FOR_NET");
			case REGISTRATION_DENIED:		return F("REG_DENIED");
			case REGISTRATION_UNKNOWN:		return F("REG_UNKNOWN");
			case ROAMING:					return F("ROAMING");
			default: return ResultToString(regStatus);
		}
		
	}*/
/*	static const __FlashStringHelper * IpStatus2Pstr(int ipStatus)
	{
		switch(ipStatus)
		{
			case IP_INITIAL:	return F("IP_INITIAL");
			case IP_START:		return F("IP_START");
			case IP_CONFIG:		return F("IP_CONFIG");
			case IP_GPRSACT:	return F("IP_GPRSACT");
			case IP_STATUS:		return F("IP_STATUS");
			case TCP_CONNECTING:return F("TCP_CONNECTING");
			case TCP_CLOSED:	return F("TCP_CLOSED");
			case PDP_DEACT:		return F("PDP_DEACT");
			case CONNECT_OK:	return F("CONNECT_OK");
			default: return ResultToString(ipStatus);
		}
	}*/
}; //Sim900Strings

#endif //__SIM900STRINGS_H__
