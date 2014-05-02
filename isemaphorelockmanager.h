
#include "common.h"
#include "signalmanager.h"

#ifndef ISEMAPHORELOCKMANAGER_H
#define ISEMAPHORELOCKMANAGER_H

typedef struct SISemaphoreLockManager
{
	IErrorLogger       errorLogger;
    ISignalManager     signalManager; 

	void (*signalCtor)(void* self);

	void (*dispatchQueuedSignals)();

	void (*lockSemaphore)(
        void*          self,
	    TSemaphore     sem);

	void (*unlockSemaphore)(
        void*          self,
	    TSemaphore     sem);

} SISemaphoreLockManager, *ISemaphoreLockManager;

#endif