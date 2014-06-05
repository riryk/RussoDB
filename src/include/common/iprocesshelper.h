#ifndef IPROCESSHELPER_H
#define IPROCESSHELPER_H

#include "ierrorlogger.h"
#include "proc.h"

typedef struct SIProcessManager
{
	IErrorLogger   errorLogger;

	TProcess (*startSubProcess)(void* self, int argc, char* argv[]);
	int (*subProcessMain)(void* self, int argc, char* argv[]);
	void (*killAllSubProcesses)();
	Bool (*restoreBackandParams)(void* self, BackendParams param);
	BackendParams (*restoreBackendParamsFromSharedMemory)();
} SIProcessManager, *IProcessManager;

#endif

