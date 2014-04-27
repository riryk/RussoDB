
#include "common.h"
#include "ierrormanager.h"

#ifndef ISIGNALMANAGER_H
#define ISIGNALMANAGER_H

typedef struct SISignalManager
{
	IErrorLogger       errorLogger;

	void (*signalCtor)(void* self);

} SISignalManager, *ISignalManager;


#endif