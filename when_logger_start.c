
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"
#include "logger.h"
#include "processhelper.h"
#include "confmanager.h"
#include "errorlogger.h"

TEST_GROUP(logger_start);

ILogger       ll_ls;
IErrorLogger  er_ls;
char*         log_folder_ls = "logs";
char*         test_message  = "I am a large test message.";

SETUP_DEPENDENCIES(logger_start) 
{
    ll_ls = (ILogger)malloc(sizeof(SILogger));
	ll_ls->errorLogger    = &sFakeErrorLogger;
    ll_ls->memManager     = &sFakeMemManager;
    ll_ls->processManager = &sProcessManager;
    ll_ls->ctorLogger     = ctorLogger;
	ll_ls->logger_start   = logger_start;

    er_ls = (IErrorLogger)malloc(sizeof(SIErrorLogger));
	er_ls->confManager          = &sConfManager;
	er_ls->memContManager       = &sMemContainerManager;
	er_ls->writeMessageInChunks = writeMessageInChunks;
}

GIVEN(logger_start) 
{
    ll_ls->ctorLogger(ll_ls, log_folder_ls);
}

WHEN(logger_start)
{
    ll_ls->logger_start(ll_ls);

    Sleep(5000);

    er_ls->writeMessageInChunks(er_ls, test_message, strlen(test_message)); 
}

TEST_TEAR_DOWN(logger_start)
{
	int res = rmdir(log_folder_ls);
}

TEST(logger_start, then_test)
{
}

TEST_GROUP_RUNNER(logger_start)
{
    RUN_TEST_CASE(logger_start, then_test);
}


