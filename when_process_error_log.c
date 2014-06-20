
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"
#include "logger.h"
#include "processhelper.h"
#include "confmanager.h"
#include "errorlogger.h"
#include "string_info.h"
#include "error.h"

TEST_GROUP(process_error_log);

IErrorLogger    er_pel;

SETUP_DEPENDENCIES(process_error_log) 
{
    er_pel = (IErrorLogger)malloc(sizeof(SIErrorLogger));
    er_pel->confManager           = &sConfManager;
    er_pel->memContManager        = &sMemContainerManager;
	er_pel->strManager            = &sStringManager;
	er_pel->writeMessageInChunks  = writeMessageInChunks;
	er_pel->logger                = &sLogger;
	er_pel->assertArg             = assertArg;
	er_pel->assert                = assert; 
	er_pel->log                   = log;
	er_pel->writeException        = writeException;
	er_pel->beginError            = beginError;
	er_pel->endError              = endError;
	er_pel->ctorErrorLogger       = ctorErrorLogger;
}

GIVEN(process_error_log) 
{
	er_pel->ctorErrorLogger(er_pel);
}

WHEN(process_error_log)
{
	PROCESS_ERROR_LOG(er_pel, LOG_ERROR, NULL, "I am a test error message");
}

TEST_TEAR_DOWN(process_error_log)
{
	free(er_pel);
}

TEST(process_error_log, then_test)
{

}

TEST_GROUP_RUNNER(process_error_log)
{
    RUN_TEST_CASE(process_error_log, then_test);
}


