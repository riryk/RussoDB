
#include "common.h"
#include "isignalmanager.h"
#include "ierrorlogger.h"
#include "semaphore.h"

#ifndef ISEMAPHORELOCKMANAGER_H
#define ISEMAPHORELOCKMANAGER_H

typedef struct SISemaphoreLockManager
{
	IErrorLogger       errorLogger;
    ISignalManager     signalManager; 

	void (*signalCtor)(void* self);

	void (*dispatchQueuedSignals)();

	void (*semaphoreCreate)(
        void*          self,
	    TSemaphore     sem);

	void (*semaphoresCtor)(
	    void*          self,
	    int            semasMax, 
	    int            port);

	void (*lockSemaphore)(
        void*          self,
	    TSemaphore     sem);

	void (*unlockSemaphore)(
        void*          self,
	    TSemaphore     sem);

	void (*releaseSemaphores)();

	Bool (*tryLockSemaphore)(
        void*          self,
        TSemaphore     sem);

} SISemaphoreLockManager, *ISemaphoreLockManager;

#endif
