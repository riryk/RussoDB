
#include "common.h"
#include "ierrorlogger.h"

#ifndef ISIGNALMANAGER_H
#define ISIGNALMANAGER_H

typedef struct SISignalManager
{
	IErrorLogger       errorLogger;

	void (*signalCtor)(void* self);
	void (*dispatchQueuedSignals)();

} SISignalManager, *ISignalManager;


#endif