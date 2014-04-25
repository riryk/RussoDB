
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"
#include "processhelper.h"

TEST_GROUP(sub_proc_start);

IProcessManager  pm_ssc;
char*            pm_args[4];

SETUP_DEPENDENCIES(sub_proc_start) 
{
    pm_ssc = (IProcessManager)malloc(sizeof(SIProcessManager));
	pm_ssc->errorLogger         = &sFakeErrorLogger;
	pm_ssc->startSubProcess     = startSubProcess;
	pm_ssc->killAllSubProcesses = killAllSubProcesses;
}

GIVEN(sub_proc_start) 
{
    pm_args[0] = "---";
    pm_args[1] = "subproc";
    pm_args[2] = NULL;
    pm_args[3] = NULL;
}

WHEN(sub_proc_start)
{
	pm_ssc->startSubProcess(pm_ssc, 0, pm_args);
}

TEST_TEAR_DOWN(sub_proc_start) 
{
    pm_ssc->killAllSubProcesses();
}

TEST(sub_proc_start, then_test)
{
}

TEST_GROUP_RUNNER(sub_proc_start)
{
    RUN_TEST_CASE(sub_proc_start, then_test);
}

