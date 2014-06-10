
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"
#include "logger.h"
#include "processhelper.h"
#include "confmanager.h"
#include "errorlogger.h"

TEST_GROUP(logger_start);

ILogger         ll_ls;
IErrorLogger    er_ls;
IProcessManager proc_ls;

char*           log_folder_ls = "logs";
char*           test_message  = "I am a large test message.";
FILE*           log_file_ls   = NULL;

BackendParams   param_ls; 
char*           log_file_name_ls;

SETUP_DEPENDENCIES(logger_start) 
{
    ll_ls = (ILogger)malloc(sizeof(SILogger));
	ll_ls->errorLogger    = &sFakeErrorLogger;
    ll_ls->memManager     = &sFakeMemManager;
    ll_ls->processManager = &sProcessManager;
    ll_ls->ctorLogger     = ctorLogger;
	ll_ls->logger_start   = logger_start;
    ll_ls->getLogFileName = getLogFileName;
	ll_ls->logFileOpen    = logFileOpen;

    er_ls = (IErrorLogger)malloc(sizeof(SIErrorLogger));
	er_ls->confManager          = &sConfManager;
	er_ls->memContManager       = &sMemContainerManager;
	er_ls->writeMessageInChunks = writeMessageInChunks;

	proc_ls = (IProcessManager)malloc(sizeof(SIProcessManager));
	proc_ls->errorLogger          = &sFakeErrorLogger;
	proc_ls->memManager           = &sFakeMemManager;
	proc_ls->startSubProcess      = startSubProcess;
    proc_ls->subProcessMain       = subProcessMain;
    proc_ls->killAllSubProcesses  = killAllSubProcesses;
    proc_ls->restoreBackandParams = restoreBackandParams;
    proc_ls->restoreBackendParamsFromSharedMemory = restoreBackendParamsFromSharedMemory;
    proc_ls->fillBackandParams    = fillBackandParams;
}

GIVEN(logger_start) 
{
	IMemoryManager  mem_man = (IMemoryManager)ll_ls->memManager;
    
	param_ls = (BackendParams)mem_man->alloc(sizeof(SBackendParams));

    ll_ls->ctorLogger(ll_ls, log_folder_ls);

	proc_ls->fillBackandParams(proc_ls, param_ls, NULL, -1); 

	log_file_name_ls = ll_ls->getLogFileName(ll_ls, 0);
}

WHEN(logger_start)
{
    ll_ls->logger_start(ll_ls);

    Sleep(5000);

    er_ls->writeMessageInChunks(er_ls, test_message, strlen(test_message)); 

	Sleep(5000);
}

TEST_TEAR_DOWN(logger_start)
{
	int res = rmdir(log_folder_ls);

	proc_ls->killAllSubProcesses();
	proc_ls->memManager->freeAll();

	free(proc_ls);
    free(ll_ls);
    free(er_ls);
}

TEST(logger_start, then_test)
{
	size_t  read_size;
	size_t  buf_size = 1024;
    char    buf_dest[1024];

	log_file_ls = ll_ls->logFileOpen(ll_ls, log_file_name_ls, "a");

	read_size   = fread(buf_dest, buf_size, buf_size, log_file_ls);

	fclose(log_file_ls);
}

TEST_GROUP_RUNNER(logger_start)
{
    RUN_TEST_CASE(logger_start, then_test);
}


