#pragma once
#include "mex.h"

/* Runtime assertion macro */

#define ERROR2( AssertMsg, Msg )        \
{                                       \
    mexWarnMsgTxt(AssertMsg);			\
	mexErrMsgTxt(Msg);					\
}	

#define ERROR( Msg )					\
{                                       \
	mexErrMsgTxt(Msg);					\
}										\

#define ASSERT( Condition )											\
{                                                                   \
    if( !(Condition) )                                              \
        ERROR( "Assertion: " #Condition " failed");					\
}

#define ASSERT2( Condition , userMessage)                           \
{                                                                   \
    if( !(Condition) )                                              \
        ERROR2( "Assertion: " #Condition " failed" , userMessage);  \
}

#ifdef _DEBUG
#define DEBUG_ASSERT(Condition)					ASSERT( Condition )	
#define DEBUG_ASSERT2(Condition, userMessage)	ASSERT( Condition, userMessage )	
#else
#define DEBUG_ASSERT(Condition)
#define DEBUG_ASSERT2(Condition, userMessage)
#endif
