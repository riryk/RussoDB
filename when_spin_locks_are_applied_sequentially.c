#include "unity_fixture.h"
#include "fakeerrorlogger.h"
#include "spinlockmanager.h"
#include "fakememmanager.h"
#include <windows.h>
#include "spin.h"
#include "threadhelper.h"

TEST_GROUP(spin_locks_are_applied_sequentially);

ISpinLockManager  m_slaas;
IThreadHelper     th_slaas;
TSpinLock         slock_slaas;
int               sleeps_count_slaas;
int               thread_2_sleep_time_slaas;
TThread           threadHandle_slaas_1;
TThread           threadHandle_slaas_2;
TThread           threadHandle_slaas_3;
TThread           threadHandle_slaas_4;
int               spinsAllowedCount_slaas_1;
int               spinsAllowedCount_slaas_2;
int               spinsAllowedCount_slaas_3;
int               spinsAllowedCount_slaas_4;
TThreadId         thread_id_slaas;

#ifdef _WIN32

DWORD WINAPI spinLockFunc_slaas(LPVOID lpParam) 
{
	SPIN_LOCK_ACQUIRE(m_slaas, &slock_slaas);

	Sleep(thread_2_sleep_time_slaas * 1000L);

	SPIN_LOCK_RELEASE(m_slaas, &slock_slaas);
}

#endif

void sleep_slaas(int sleepMilliseconds)
{
	Sleep(sleepMilliseconds);
}

SETUP_DEPENDENCIES(spin_locks_are_applied_sequentially) 
{
    m_slaas = (ISpinLockManager)malloc(sizeof(SISpinLockManager));
    m_slaas->spinLockCtor    = &spinLockCtor;
    m_slaas->errorLogger     = &sFakeErrorLogger;
	m_slaas->memManager      = &sFakeMemManager;
    m_slaas->spinLockAcquire = spinLockAcquire;
    m_slaas->spinLockRelease = spinLockRelease;

	th_slaas = (IThreadHelper)malloc(sizeof(SIThreadHelper));
	th_slaas->threadHelpCtor = &threadHelpCtor;
    th_slaas->errorLogger    = &sFakeErrorLogger;
    th_slaas->startThread    = startThread;
}

GIVEN(spin_locks_are_applied_sequentially) 
{
    slock_slaas               = 0;
	thread_2_sleep_time_slaas = 10;

	m_slaas->spinLockCtor(m_slaas, sleep_slaas);
}

WHEN(spin_locks_are_applied_sequentially)
{
	threadHandle_slaas_1 = th_slaas->startThread(
		     th_slaas, 
			 spinLockFunc_slaas, 
			 NULL, 
			 thread_id_slaas);

    Sleep(2 * 1000L);
    SPIN_LOCK_ACQUIRE(m_slaas, &slock_slaas);
	spinsAllowedCount_slaas_1 = spinsAllowedCount;
    SPIN_LOCK_RELEASE(m_slaas, &slock_slaas);

    threadHandle_slaas_2 = th_slaas->startThread(
		     th_slaas, 
			 spinLockFunc_slaas, 
			 NULL, 
			 thread_id_slaas);

    Sleep(2 * 1000L);
    SPIN_LOCK_ACQUIRE(m_slaas, &slock_slaas);
	spinsAllowedCount_slaas_2 = spinsAllowedCount;
    SPIN_LOCK_RELEASE(m_slaas, &slock_slaas);

    threadHandle_slaas_3 = th_slaas->startThread(
		     th_slaas, 
			 spinLockFunc_slaas, 
			 NULL, 
			 thread_id_slaas);

    Sleep(2 * 1000L);
    SPIN_LOCK_ACQUIRE(m_slaas, &slock_slaas);
	spinsAllowedCount_slaas_3 = spinsAllowedCount;
    SPIN_LOCK_RELEASE(m_slaas, &slock_slaas);

    threadHandle_slaas_4 = th_slaas->startThread(
		     th_slaas, 
			 spinLockFunc_slaas, 
			 NULL, 
			 thread_id_slaas);

	Sleep(2 * 1000L);
	SPIN_LOCK_ACQUIRE(m_slaas, &slock_slaas);
	spinsAllowedCount_slaas_4 = spinsAllowedCount;
	SPIN_LOCK_RELEASE(m_slaas, &slock_slaas);
}

TEST_TEAR_DOWN(spin_locks_are_applied_sequentially)
{
	SPIN_LOCK_RELEASE(m_slaas, &slock_slaas);

#ifdef _WIN32

    TerminateThread(threadHandle_slaas_1, 0); 
    CloseHandle(threadHandle_slaas_1);		 

    TerminateThread(threadHandle_slaas_2, 0); 
    CloseHandle(threadHandle_slaas_2);		 
    
	TerminateThread(threadHandle_slaas_3, 0); 
    CloseHandle(threadHandle_slaas_3);		 

	TerminateThread(threadHandle_slaas_4, 0); 
    CloseHandle(threadHandle_slaas_4);		 

#endif

	m_slaas->memManager->freeAll();

	free(m_slaas);
	free(th_slaas);
}

TEST(spin_locks_are_applied_sequentially, then_spins_allowed_count_must_decrease)
{
    TEST_ASSERT_NOT_NULL(m_slaas);

	TEST_ASSERT_EQUAL_UINT32(spinsAllowedCount_slaas_1, 100 - 1);
    TEST_ASSERT_EQUAL_UINT32(spinsAllowedCount_slaas_2, 100 - 2);
    TEST_ASSERT_EQUAL_UINT32(spinsAllowedCount_slaas_3, 100 - 3);
    TEST_ASSERT_EQUAL_UINT32(spinsAllowedCount_slaas_4, 100 - 4);
}

TEST_GROUP_RUNNER(spin_locks_are_applied_sequentially)
{
    RUN_TEST_CASE(spin_locks_are_applied_sequentially, then_spins_allowed_count_must_decrease);
}


