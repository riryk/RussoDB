
#include "ithreadhelper.h"

#ifndef THREADHELPER_H
#define THREADHELPER_H

extern sleepFunc  slpFunc; 

void threadHelpCtor(sleepFunc slpFuncParam);

TThread startThread(
    void*            self,
	THREAD_FUNC      func, 
	void*            param, 
	TThreadId        threadid);

#endif