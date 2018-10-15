#include "FunctionBase.h"

#include "../Sim900Constants.h"


// default constructor
FunctionBase::FunctionBase()
{
	IsReady = false;
	functionTimeout = AT_DEFAULT_TIMEOUT;
} //FunctionBase