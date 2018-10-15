/* 
* FunctionBase.cpp
*
* Created: 2014-03-21 18:42:22
* Author: Tom
*/


#include "FunctionBase.h"

// default constructor
FunctionBase::FunctionBase()
{
	IsReady = false;
	functionTimeout = AT_DEFAULT_TIMEOUT;
} //FunctionBase