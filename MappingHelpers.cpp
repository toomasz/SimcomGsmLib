#include "MappingHelpers.h"



#define SEARCHING_FOR_NETWORK0 0
#define HOME_NETWORK 1
#define SEARCHING_FOR_NETWORK 2
#define REGISTRATION_DENIED 3
#define REGISTRATION_UNKNOWN 4
#define ROAMING 5

GsmNetworkStatus CregToNetworkStatus(uint16_t status)
{
	switch (status)
	{
	case 0:
	case 2:
		return GsmNetworkStatus::SearchingForNetwork;
	case 1:
		return GsmNetworkStatus::HomeNetwork;
	case 3:
		return GsmNetworkStatus::RegistrationDenied;
	case 4:
		return GsmNetworkStatus::RegistrationUnknown;
	case 5:
		return GsmNetworkStatus::Roaming;
	default:
		return GsmNetworkStatus::RegistrationUnknown;

	}
}


