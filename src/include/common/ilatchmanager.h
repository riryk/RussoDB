
#include <stdlib.h>
#include <string.h>
#include "latch.h"
#include "ierrorlogger.h"
#include "imemorymanager.h"
#include "signalmanager.h"

#ifndef ILATCHMANAGER_H
#define ILATCHMANAGER_H

typedef struct SILatchManager
{
	IMemoryManager   memManager;
	IErrorLogger     errorLogger;
	ISignalManager   signalManager;

	Latch (*initLatch) (void* self);
	void  (*setLatch)  (Latch latch);
	void  (*resetLatch)(Latch latch);
	void  (*waitLatch) (void* self, Latch latch);
} SILatchManager, *ILatchManager;

#endif