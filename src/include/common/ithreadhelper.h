
#ifndef ITHREADHELPER_H
#define ITHREADHELPER_H

#include "ierrorlogger.h"
#include "thread.h"

typedef struct SIThreadHelper
{
	IErrorLogger   errorLogger;

	TThread (*startThread)(
         void*            self,
	     ThreadFunc       func, 
	     void*            param, 
	     TThreadId        threadid);

} SIThreadHelper, *IThreadHelper;

#endif

