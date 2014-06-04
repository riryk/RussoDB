
#include <stdlib.h>
#include <string.h>
#include "latch.h"
#include "ierrorlogger.h"
#include "imemorymanager.h"

#ifndef ILATCHMANAGER_H
#define ILATCHMANAGER_H

typedef struct SILatchManager
{
	IMemoryManager   memManager;
	IErrorLogger     errorLogger;

	void (*initLatch)(void* self, Latch latch);
	void (*setLatch)(Latch latch);
	void (*resetLatch)(Latch latch);
} SILatchManager, *ILatchManager;

#endif