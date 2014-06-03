
#include "common.h"
#include "ierrorlogger.h"
#include "signal.h"

#ifndef ISIGNALMANAGER_H
#define ISIGNALMANAGER_H

typedef struct SISignalManager
{
	IErrorLogger       errorLogger;

	void (*signalCtor)(void* self);
	void (*dispatchQueuedSignals)();
	void (*signalDtor)(void* self);

	signalFunc (*setSignal)(
         void*          self,
         int            signum, 
	     signalFunc     handler);

	void (*queueSignal)(int signum);

	int (*sentSignal)(
	     void*          self, 
	     int            pid,
	     int            signal);

} SISignalManager, *ISignalManager;


#endif