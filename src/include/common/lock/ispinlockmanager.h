
#include "common.h"
#include "ierrorlogger.h"
#include "imemorymanager.h"
#include "thread.h"

#ifndef ISPINLOCKMANAGER_H
#define ISPINLOCKMANAGER_H

typedef struct SISpinLockManager
{
	IErrorLogger   errorLogger;
    IMemoryManager memManager;

	void (*spinLockCtor)(
         void*             self,
         sleepFunc         slpFuncParam);

	int (*spinLockAcquire)(
	     void*             self,
	     volatile long*    lock, 
	     char*             file, 
	     int               line);

	void (*spinLockRelease)(volatile long* lock);

} SISpinLockManager, *ISpinLockManager;

#endif

