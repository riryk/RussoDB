
#include "ithreadhelper.h"

#ifndef THREADHELPER_H
#define THREADHELPER_H

extern sleepFunc  slpFunc; 
extern TEvent     threadStartEvent;

void threadHelpCtor(
    void*            self,
    sleepFunc        slpFuncParam,
	Bool             includeStartThreadEvent);

TThread startThread(
    void*            self,
	THREAD_FUNC      func, 
	void*            param, 
	TThreadId        threadid);

void spinWait(int spinMaxCount);

void waitForEvent(
	void*       self,
	TEvent      eventToWait);

#endif