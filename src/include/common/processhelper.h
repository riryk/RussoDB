#include "common.h"
#include <stdlib.h>
#include "iprocesshelper.h"
#include "logger.h"
#include "filemanager.h"
#include "semaphorelockmanager.h"
#include "proc.h"

#ifndef PROCESS_HELPER_H
#define PROCESS_HELPER_H

extern const SIProcessManager sProcessManager;
extern const IProcessManager  processManager;
extern ProcBackData           backendProc;

int startSubProcess(void* self, int argc, char* argv[]);
int subProcessMain(void* self, int argc, char* argv[]);
void killAllSubProcesses();

#endif


