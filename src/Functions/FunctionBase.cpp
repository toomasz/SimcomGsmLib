#include "FunctionBase.h"

#include "../GsmLibConstants.h"


// default constructor
FunctionBase::FunctionBase()
{
	IsReady = false;
	functionTimeout = AT_DEFAULT_TIMEOUT;
} //FunctionBase