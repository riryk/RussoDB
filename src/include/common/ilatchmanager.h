
#include <stdlib.h>
#include <string.h>
#include "latch.h"


#ifndef ILATCHMANAGER_H
#define ILATCHMANAGER_H

typedef struct SILatchManager
{
	void (*setLatch)(Latch latch);
} SILatchManager, *ILatchManager;

#endif