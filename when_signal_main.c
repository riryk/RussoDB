#include "unity_fixture.h"
#include "fakeerrorlogger.h"
#include "fakememmanager.h"
#include "signalmanager.h"
#include "snprintf.h"
#include <windows.h>

TEST_GROUP(signal_main);

ISignalManager    sm_sm;

SETUP_DEPENDENCIES(signal_main) 
{
    sm_sm = (ISignalManager)malloc(sizeof(SISignalManager));
	sm_sm->errorLogger           = &sFakeErrorLogger;
	sm_sm->signalCtor            = signalCtor;
	sm_sm->dispatchQueuedSignals = dispatchQueuedSignals;
}

GIVEN(signal_main) 
{
	sm_sm->signalCtor(sm_sm);
	Sleep(1000000);
}

WHEN(signal_main)
{
}

TEST_TEAR_DOWN(signal_main)
{
	free(sm_sm);
}

TEST(signal_main, then_test)
{
}

TEST_GROUP_RUNNER(signal_main)
{
    RUN_TEST_CASE(signal_main, then_test);
}


