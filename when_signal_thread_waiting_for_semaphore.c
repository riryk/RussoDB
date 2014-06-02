#include "unity_fixture.h"
#include "fakeerrorlogger.h"
#include "fakememmanager.h"
#include "signalmanager.h"
#include "snprintf.h"
#include "threadhelper.h"
#include "semaphorelockmanager.h"
#include <windows.h>

TEST_GROUP(signal_thread_waiting_for_semaphore);

ISignalManager        sm_stwfs;
IThreadHelper         th_stwfs;
ISemaphoreLockManager slm_stwfs;

TThread        thread_working_stwfs;
TThread        thread_error_stwfs;
TEvent         event_stwfs;
TSemaphore     sem_stwfs; 

int            signal_interrupt_func_calls_stwfs   = 0;

SETUP_DEPENDENCIES(signal_thread_waiting_for_semaphore) 
{
    sm_stwfs = (ISignalManager)malloc(sizeof(SISignalManager));
	sm_stwfs->errorLogger           = &sFakeErrorLogger;
	sm_stwfs->signalCtor            = signalCtor;
	sm_stwfs->dispatchQueuedSignals = dispatchQueuedSignals;
	sm_stwfs->signalDtor            = signalDtor;
	sm_stwfs->setSignal             = setSignal;
    sm_stwfs->queueSignal           = queueSignal;

	th_stwfs = (IThreadHelper)malloc(sizeof(SIThreadHelper));
	th_stwfs->threadHelpCtor        = &threadHelpCtor;
    th_stwfs->errorLogger           = &sFakeErrorLogger;
    th_stwfs->startThread           = startThread;
	th_stwfs->waitForEvent          = waitForEvent;
	th_stwfs->waitForMultipleEvents = waitForMultipleEvents;

	slm_stwfs = (ISemaphoreLockManager)malloc(sizeof(SISemaphoreLockManager));
    slm_stwfs->errorLogger          = &sFakeErrorLogger;
    slm_stwfs->semaphoresCtor       = semaphoresCtor;
    slm_stwfs->semaphoreCreate      = semaphoreCreate;
	slm_stwfs->lockSemaphore        = lockSemaphore;
}

void signal_interrupt_func_stwfs  () 
{ 
	HANDLE currentThread = GetCurrentThread(); 
	signal_interrupt_func_calls_stwfs++; 
    
	TerminateThread(currentThread, 0); 
}

#ifdef _WIN32

DWORD WINAPI workingThreadFunc_stwfs(LPVOID lpParam) 
{
    Sleep(1000);

	slm_stwfs->lockSemaphore(slm_stwfs, sem_stwfs);
}

DWORD WINAPI errorThreadFunc_stwfs(LPVOID lpParam) 
{
    Sleep(100000);
    
	sm_stwfs->queueSignal(SIGNAL_INTERRUPT);
}

#endif

GIVEN(signal_thread_waiting_for_semaphore) 
{
	sm_stwfs->signalCtor(sm_stwfs);
	sm_stwfs->setSignal(sm_stwfs, SIGNAL_INTERRUPT, signal_interrupt_func_stwfs);
}

WHEN(signal_thread_waiting_for_semaphore)
{
	TEvent  eventsToWait[2];

    slm_stwfs->semaphoresCtor (slm_stwfs, 100, 0);
    slm_stwfs->semaphoreCreate(slm_stwfs, sem_stwfs);
	slm_stwfs->lockSemaphore  (slm_stwfs, sem_stwfs);

	event_stwfs 
		= CreateEvent(
		     NULL, 
			 False, 
			 False,
			 NULL);

    thread_working_stwfs 
		= th_stwfs->startThread(
		     th_stwfs, 
			 workingThreadFunc_stwfs, 
			 NULL, 
			 thread_working_stwfs); 	

	thread_error_stwfs 
		= th_stwfs->startThread(
		     th_stwfs, 
			 errorThreadFunc_stwfs, 
			 NULL, 
			 thread_error_stwfs); 	

	eventsToWait[0] = thread_working_stwfs;
    eventsToWait[1] = thread_error_stwfs;

	th_stwfs->waitForMultipleEvents(th_stwfs, eventsToWait, 2, True);
}

TEST_TEAR_DOWN(signal_thread_waiting_for_semaphore)
{
    CloseHandle(thread_working_stwfs);

    TerminateThread(thread_error_stwfs, 0); 
    CloseHandle(thread_error_stwfs);

	sm_stwfs->signalDtor(sm_stwfs);
	free(sm_stwfs);
}

TEST(signal_thread_waiting_for_semaphore, then_test)
{
	TEST_ASSERT_EQUAL_INT(1, 1);
}

TEST_GROUP_RUNNER(signal_thread_waiting_for_semaphore)
{
    RUN_TEST_CASE(signal_thread_waiting_for_semaphore, then_test);
}


