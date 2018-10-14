#ifndef _GSMDEBUGHELPERS_H
#define _GSMDEBUGHELPERS_H

#include <Arduino.h>
#include "ParserSim900.h"
#include <pgmspace.h>
#include "Sim900Constants.h"

const __FlashStringHelper* IpStatusToStr(SimcomIpState ipStatus)
{
	switch (ipStatus)
	{
	case SimcomIpState::IpInitial:	return F("IP_INITIAL");
	case SimcomIpState::IpStart:	return F("IP_START");
	case SimcomIpState::IpConfig:	return F("IP_CONFIG");
	case SimcomIpState::IpGprsact:	return F("IP_GPRSACT");
	case SimcomIpState::IpStatus:	return F("IP_STATUS");
	case SimcomIpState::TcpConnecting:return F("TCP_CONNECTING");
	case SimcomIpState::TcpClosed:	return F("TCP_CLOSED");
	case SimcomIpState::PdpDeact:	return F("PDP_DEACT");
	case SimcomIpState::ConnectOk:	return F("CONNECT_OK");
	default: return F("Unknown");
	}
}	

#endif