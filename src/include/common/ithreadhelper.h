
#ifndef ITHREADHELPER_H
#define ITHREADHELPER_H

#include "ierrorlogger.h"
#include "thread.h"

typedef struct SIThreadHelper
{
	IErrorLogger   errorLogger;
    
	void (*threadHelpCtor)(sleepFunc slpFuncParam);

	TThread (*startThread)(
         void*            self,
	     THREAD_FUNC      func, 
	     void*            param, 
	     TThreadId        threadid);

} SIThreadHelper, *IThreadHelper;

#endif

