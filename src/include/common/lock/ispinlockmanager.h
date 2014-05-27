
#include "common.h"
#include "ierrorlogger.h"
#include "imemorymanager.h"
#include "thread.h"
#include "spin.h"
#include "ispinlockstrategy.h"

#ifndef ISPINLOCKMANAGER_H
#define ISPINLOCKMANAGER_H

typedef struct SISpinLockManager
{
	IErrorLogger       errorLogger;
    IMemoryManager     memManager;
	ISpinLockStrategy  slockStrategy;

	void (*spinLockCtor)(
         void*              self,
         sleepFunc          slpFuncParam);

	int (*spinLockAcquire)(
	     void*              self,
	     volatile long*     lock, 
	     char*              file, 
	     int                line);

	void (*spinLockRelease)(
         void*              self,
         volatile long*     lock);

} SISpinLockManager, *ISpinLockManager;


#endif

