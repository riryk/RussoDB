#include "unity_fixture.h"
#include "fakeerrorlogger.h"
#include "fakememmanager.h"
#include "signalmanager.h"
#include "snprintf.h"
#include "threadhelper.h"
#include <windows.h>

TEST_GROUP(signal_main);

ISignalManager    sm_sm;
IThreadHelper     th_sm;

TThread           thread_working_sm;
TThread           thread_error_sm;
TEvent            event_sm;

int               work_thread_progress = 0;

int signal_hup_func_calls_sm         = 0;
int signal_interrupt_func_calls_sm   = 0;
int signal_termination_func_calls_sm = 0;
int signal_quit_func_calls_sm        = 0;
int signal_alarm_func_calls_sm       = 0;
int signal_pipe_func_calls_sm        = 0;
int signal_usr1_func_calls_sm        = 0;
int signal_usr2_func_calls_sm        = 0;
int signal_float_point_func_calls_sm = 0;
int signal_chld_func_calls_sm        = 0;

SETUP_DEPENDENCIES(signal_main) 
{
    sm_sm = (ISignalManager)malloc(sizeof(SISignalManager));
	sm_sm->errorLogger           = &sFakeErrorLogger;
	sm_sm->signalCtor            = signalCtor;
	sm_sm->dispatchQueuedSignals = dispatchQueuedSignals;
	sm_sm->signalDtor            = signalDtor;
	sm_sm->setSignal             = setSignal;
    sm_sm->queueSignal           = queueSignal;

	th_sm = (IThreadHelper)malloc(sizeof(SIThreadHelper));
	th_sm->threadHelpCtor        = &threadHelpCtor;
    th_sm->errorLogger           = &sFakeErrorLogger;
    th_sm->startThread           = startThread;
	th_sm->waitForEvent          = waitForEvent;
	th_sm->waitForMultipleEvents = waitForMultipleEvents;
}

void signal_hup_func_sm        () { signal_hup_func_calls_sm++; }
void signal_interrupt_func_sm  () 
{ 
	HANDLE currentThread = GetCurrentThread(); 
	signal_interrupt_func_calls_sm++; 
    
	TerminateThread(currentThread, 0); 
}

void signal_termination_func_sm() { signal_termination_func_calls_sm++; }
void signal_quit_func_sm       () { signal_quit_func_calls_sm++; }
void signal_alarm_func_sm      () { signal_alarm_func_calls_sm++; }
void signal_pipe_func_sm       () { signal_pipe_func_calls_sm++; }
void signal_usr1_func_sm       () { signal_usr1_func_calls_sm++; }
void signal_usr2_func_sm       () { signal_usr2_func_calls_sm++; }
void signal_float_point_func_sm() { signal_float_point_func_calls_sm++; }
void signal_chld_func_sm       () { signal_chld_func_calls_sm++; }

#ifdef _WIN32

DWORD WINAPI workingThreadFunc_sm(LPVOID lpParam) 
{
	int    i;
    
	for (i = 0; i < 1000; i++)
	{
        work_thread_progress = i; 

        if (i == 500)
		{
			th_sm->waitForEvent(th_sm, event_sm);
			PROCESS_INTERRUPTS(sm_sm);
		}
	}
}

DWORD WINAPI errorThreadFunc_sm(LPVOID lpParam) 
{
    Sleep(1000);
    
	sm_sm->queueSignal(SIGNAL_INTERRUPT);
	SetEvent(event_sm);
}

#endif

GIVEN(signal_main) 
{
	sm_sm->signalCtor(sm_sm);

	sm_sm->setSignal(sm_sm, SIGNAL_HUP,             signal_hup_func_sm);
	sm_sm->setSignal(sm_sm, SIGNAL_INTERRUPT,       signal_interrupt_func_sm);
    sm_sm->setSignal(sm_sm, SIGNAL_TERMINATION,     signal_termination_func_sm); 
    sm_sm->setSignal(sm_sm, SIGNAL_QUIT,            signal_quit_func_sm); 
    sm_sm->setSignal(sm_sm, SIGNAL_ALRM,            signal_alarm_func_sm);
	sm_sm->setSignal(sm_sm, SIGNAL_PIPE,            signal_pipe_func_sm);
    sm_sm->setSignal(sm_sm, SIGNAL_USR1,            signal_usr1_func_sm);
    sm_sm->setSignal(sm_sm, SIGNAL_USR2,            signal_usr2_func_sm);
	sm_sm->setSignal(sm_sm, SIGNAL_FLOAT_POINT_EXC, signal_float_point_func_sm);
    sm_sm->setSignal(sm_sm, SIGNAL_CHLD,            signal_chld_func_sm);
}

WHEN(signal_main)
{
	TEvent  eventsToWait[2];

	event_sm 
		= CreateEvent(
		     NULL, 
			 False, 
			 False,
			 NULL);

    thread_working_sm 
		= th_sm->startThread(
		     th_sm, 
			 workingThreadFunc_sm, 
			 NULL, 
			 thread_working_sm); 	

	thread_error_sm 
		= th_sm->startThread(
		     th_sm, 
			 errorThreadFunc_sm, 
			 NULL, 
			 thread_error_sm); 	

	eventsToWait[0] = thread_working_sm;
    eventsToWait[1] = thread_error_sm;

	th_sm->waitForMultipleEvents(th_sm, eventsToWait, 2, True);
}

TEST_TEAR_DOWN(signal_main)
{
    CloseHandle(thread_working_sm);

    TerminateThread(thread_error_sm, 0); 
    CloseHandle(thread_error_sm);

	sm_sm->signalDtor(sm_sm);
	free(sm_sm);
}

TEST(signal_main, then_signal_interrupt_func_must_be_called)
{
	TEST_ASSERT_EQUAL_INT(signal_interrupt_func_calls_sm, 1);
}

TEST(signal_main, then_working_thread_should_stop_working)
{
	TEST_ASSERT_EQUAL_INT(work_thread_progress, 500);
}

TEST_GROUP_RUNNER(signal_main)
{
    RUN_TEST_CASE(signal_main, then_signal_interrupt_func_must_be_called);
    RUN_TEST_CASE(signal_main, then_working_thread_should_stop_working);
}


