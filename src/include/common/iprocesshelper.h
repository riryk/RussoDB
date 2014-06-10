#ifndef IPROCESSHELPER_H
#define IPROCESSHELPER_H

#include "ierrorlogger.h"
#include "proc.h"
#include "trackmemmanager.h"

typedef struct SIProcessManager
{
	IErrorLogger     errorLogger;
    IMemoryManager   memManager;

	TProcess (*startSubProcess)(void* self, int argc, char* argv[]);
	int (*subProcessMain)(void* self, int argc, char* argv[]);
	void (*killAllSubProcesses)();
	Bool (*restoreBackandParams)(void* self, BackendParams param);
	BackendParams (*restoreBackendParamsFromSharedMemory)(void* self);
	Bool (*fillBackandParams)(
           void*            self,
	       BackendParams    param, 
	       HANDLE           childProcess, 
	       int              childPid);

} SIProcessManager, *IProcessManager;

#endif

