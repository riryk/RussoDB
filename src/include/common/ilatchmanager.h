
#include <stdlib.h>
#include <string.h>
#include "latch.h"
#include "ierrorlogger.h"

#ifndef ILATCHMANAGER_H
#define ILATCHMANAGER_H

typedef struct SILatchManager
{
	IErrorLogger  errorLogger;

	void (*initLatch)(void* self, Latch latch);
	void (*setLatch)(Latch latch);
} SILatchManager, *ILatchManager;

#endif