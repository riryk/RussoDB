
#ifndef ITHREADHELPER_H
#define ITHREADHELPER_H

#include "ierrorlogger.h"
#include "thread.h"

typedef struct SIThreadHelper
{
	IErrorLogger   errorLogger;

	void (*threadHelpCtor)(
         void*            self,
         sleepFunc        slpFuncParam,
	     Bool             includeStartThreadEvent);

	TThread (*startThread)(
         void*            self,
	     THREAD_FUNC      func, 
	     void*            param, 
	     TThreadId        threadid);

	void (*spinWait)(int spinMaxCount);

	void (*waitForEvent)(
	     void*            self,
	     TEvent           eventToWait);

} SIThreadHelper, *IThreadHelper;

#endif

