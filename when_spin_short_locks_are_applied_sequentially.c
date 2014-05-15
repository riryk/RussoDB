#include "unity_fixture.h"
#include "fakeerrorlogger.h"
#include "spinlockmanager.h"
#include "fakememmanager.h"
#include <windows.h>
#include "spin.h"
#include "threadhelper.h"

TEST_GROUP(spin_short_locks_are_applied_sequentially);

ISpinLockManager  m_sslaas;
IThreadHelper     th_sslaas;
TSpinLock         slock_sslaas;
int               sleeps_count_sslaas;
int               thread_2_spin_count_sslaas;
TThread           threadHandle_sslaas_1;
TThread           threadHandle_sslaas_2;
TThread           threadHandle_sslaas_3;
TThread           threadHandle_sslaas_4;
int               spinsAllowedCount_sslaas_1;
int               spinsAllowedCount_sslaas_2;
int               spinsAllowedCount_sslaas_3;
int               spinsAllowedCount_sslaas_4;
TThreadId         thread_id_sslaas;

#ifdef _WIN32

DWORD WINAPI spinLockFunc_sslaas(LPVOID lpParam) 
{
	int  i = 0; 

	SPIN_LOCK_ACQUIRE(m_sslaas, &slock_sslaas);

    if (threadStartEvent != NULL)
        SetEvent(threadStartEvent);

	while (i++ < thread_2_spin_count_sslaas)
		;

	SPIN_LOCK_RELEASE(m_sslaas, &slock_sslaas);
}

#endif

void sleep_sslaas(int sleepMilliseconds)
{
	Sleep(sleepMilliseconds);
}

SETUP_DEPENDENCIES(spin_short_locks_are_applied_sequentially) 
{
    m_sslaas = (ISpinLockManager)malloc(sizeof(SISpinLockManager));
    m_sslaas->spinLockCtor    = &spinLockCtor;
    m_sslaas->errorLogger     = &sFakeErrorLogger;
	m_sslaas->memManager      = &sFakeMemManager;
    m_sslaas->spinLockAcquire = spinLockAcquire;
    m_sslaas->spinLockRelease = spinLockRelease;

	th_sslaas = (IThreadHelper)malloc(sizeof(SIThreadHelper));
	th_sslaas->threadHelpCtor = &threadHelpCtor;
    th_sslaas->errorLogger    = &sFakeErrorLogger;
    th_sslaas->startThread    = startThread;
    th_sslaas->spinWait       = spinWait;
	th_sslaas->waitForEvent   = waitForEvent;
}

GIVEN(spin_short_locks_are_applied_sequentially) 
{
    slock_sslaas               = 0;
	thread_2_spin_count_sslaas = 15000;

	spinsMinNum                = 10000;
    spinsMaxNum                = 1000000;
	spinsAllowedCount          = 100000;

	m_sslaas->spinLockCtor(m_sslaas, sleep_sslaas);
	th_sslaas->threadHelpCtor(th_sslaas, sleep_sslaas, True);
}

WHEN(spin_short_locks_are_applied_sequentially)
{
	threadHandle_sslaas_1 = th_sslaas->startThread(
		     th_sslaas, 
			 spinLockFunc_sslaas, 
			 NULL, 
			 thread_id_sslaas);
    
	th_sslaas->waitForEvent(th_sslaas, threadStartEvent);

    SPIN_LOCK_ACQUIRE(m_sslaas, &slock_sslaas);
	spinsAllowedCount_sslaas_1 = spinsAllowedCount;
    SPIN_LOCK_RELEASE(m_sslaas, &slock_sslaas);

    threadHandle_sslaas_2 = th_sslaas->startThread(
		     th_sslaas, 
			 spinLockFunc_sslaas, 
			 NULL, 
			 thread_id_sslaas);

	th_sslaas->waitForEvent(th_sslaas, threadStartEvent);
    
    SPIN_LOCK_ACQUIRE(m_sslaas, &slock_sslaas);
	spinsAllowedCount_sslaas_2 = spinsAllowedCount;
    SPIN_LOCK_RELEASE(m_sslaas, &slock_sslaas);

    threadHandle_sslaas_3 = th_sslaas->startThread(
		     th_sslaas, 
			 spinLockFunc_sslaas, 
			 NULL, 
			 thread_id_sslaas);

	th_sslaas->waitForEvent(th_sslaas, threadStartEvent);
    
    SPIN_LOCK_ACQUIRE(m_sslaas, &slock_sslaas);
	spinsAllowedCount_sslaas_3 = spinsAllowedCount;
    SPIN_LOCK_RELEASE(m_sslaas, &slock_sslaas);

    threadHandle_sslaas_4 = th_sslaas->startThread(
		     th_sslaas, 
			 spinLockFunc_sslaas, 
			 NULL, 
			 thread_id_sslaas);

    th_sslaas->waitForEvent(th_sslaas, threadStartEvent);
	
	SPIN_LOCK_ACQUIRE(m_sslaas, &slock_sslaas);
	spinsAllowedCount_sslaas_4 = spinsAllowedCount;
	SPIN_LOCK_RELEASE(m_sslaas, &slock_sslaas);
}

TEST_TEAR_DOWN(spin_short_locks_are_applied_sequentially)
{
	SPIN_LOCK_RELEASE(m_sslaas, &slock_sslaas);

#ifdef _WIN32

    TerminateThread(threadHandle_sslaas_1, 0); 
    CloseHandle(threadHandle_sslaas_1);		 

    TerminateThread(threadHandle_sslaas_2, 0); 
    CloseHandle(threadHandle_sslaas_2);		 
    
	TerminateThread(threadHandle_sslaas_3, 0); 
    CloseHandle(threadHandle_sslaas_3);		 

	TerminateThread(threadHandle_sslaas_4, 0); 
    CloseHandle(threadHandle_sslaas_4);		 

#endif

	m_sslaas->memManager->freeAll();

	free(m_sslaas);
	free(th_sslaas);
}

TEST(spin_short_locks_are_applied_sequentially, then_spins_allowed_max_count_must_increase)
{
    TEST_ASSERT_NOT_NULL(m_sslaas);

    TEST_ASSERT_EQUAL_UINT32(spinsAllowedCount_sslaas_1, 100000 + 100);
    TEST_ASSERT_EQUAL_UINT32(spinsAllowedCount_sslaas_2, 100000 + 200);
    TEST_ASSERT_EQUAL_UINT32(spinsAllowedCount_sslaas_3, 100000 + 300);
    TEST_ASSERT_EQUAL_UINT32(spinsAllowedCount_sslaas_4, 100000 + 400);
}

TEST_GROUP_RUNNER(spin_short_locks_are_applied_sequentially)
{
    RUN_TEST_CASE(spin_short_locks_are_applied_sequentially, then_spins_allowed_max_count_must_increase);
}


