#ifndef IPROCESSMANAGER_H
#define IPROCESSMANAGER_H

#include "ierrorlogger.h"

typedef struct SIProcessManager
{
	IErrorLogger   errorLogger;

	int (*startSubProcess)(void* self, int argc, char* argv[]);
	int (*subProcessMain)(void* self, int argc, char* argv[]);
} SIProcessManager, *IProcessManager;

#endif

