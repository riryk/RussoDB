
#include "common.h"
#include "ierrormanager.h"

#ifndef ISPINLOCKMANAGER_H
#define ISPINLOCKMANAGER_H

typedef struct SISpinLockManager
{
	IErrorLogger  errorLogger;

	int (*spinLockAcquire)(
	     void*             self,
	     volatile long*    lock, 
	     char*             file, 
	     int               line);

} SISpinLockManager, *ISpinLockManager;

#endif

