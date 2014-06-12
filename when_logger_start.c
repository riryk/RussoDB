
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

char*           log_folder_ls       = "logs";
char*           test_short_message  = "I am a large test message.";
char*           test_long_message   = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
FILE*           log_file_ls         = NULL;

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
	int  short_msg_len = strlen(test_short_message);
	int  long_msg_len  = strlen(test_long_message);

    ll_ls->logger_start(ll_ls);

    Sleep(5000);

    er_ls->writeMessageInChunks(er_ls, test_short_message, short_msg_len); 
	er_ls->writeMessageInChunks(er_ls, test_long_message,  long_msg_len);

	Sleep(5000);
}

TEST_TEAR_DOWN(logger_start)
{
	int res, res_file;
	char str[100];

	proc_ls->killAllSubProcesses();

	strcpy(str, "logs\\");
	strcpy(str, log_file_name_ls);

    res_file = remove(str);
    res      = rmdir(log_folder_ls);

    TEST_ASSERT_EQUAL_INT(res, 0);

	proc_ls->memManager->freeAll();

	free(proc_ls);
    free(ll_ls);
    free(er_ls);
}

TEST(logger_start, then_the_message_should_be_written_to_the_log_file)
{
	size_t  read_size;
	size_t  buf_size = 1024;
    char    buf_dest[1024];
	int     cmp_results;
    char    str[2000];
	int     short_msg_len;

	memset(buf_dest, 0, buf_size);

	log_file_ls = ll_ls->logFileOpen(ll_ls, log_file_name_ls, "r");

	read_size   = fread(buf_dest, 1, buf_size, log_file_ls);

	fclose(log_file_ls);

	strcpy(str, test_short_message);

	short_msg_len = strlen(test_short_message);

	strcpy(str + short_msg_len, test_long_message);

	cmp_results = strcmp(str, buf_dest);

	TEST_ASSERT_EQUAL_INT(cmp_results, 0);
}

TEST_GROUP_RUNNER(logger_start)
{
    RUN_TEST_CASE(logger_start, then_the_message_should_be_written_to_the_log_file);
}


