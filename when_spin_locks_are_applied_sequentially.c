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
TThread           threadHandle_slfst_1;
TThread           threadHandle_slfst_2;
TThread           threadHandle_slfst_3;
TThread           threadHandle_slfst_4;

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
	thread_2_sleep_time_slaas = 20;

	m_slaas->spinLockCtor(m_slaas, sleep_slaas);
}

WHEN(spin_locks_are_applied_sequentially)
{
	threadHandle_slfst_1 = = th_slfst->startThread(
		     th_slfst, 
			 spinLockFunc_slfst, 
			 NULL, 
			 thread_id_slfst);

    threadHandle_slfst_2 = ;
    threadHandle_slfst_3 = ;
    threadHandle_slfst_4 = ;

	Sleep(2 * 1000L);

    sleeps_count_slmawc = SPIN_LOCK_ACQUIRE(m_slmawc, &slock_slmawc);
}

TEST_TEAR_DOWN(spin_locks_are_applied_sequentially)
{
	SPIN_LOCK_RELEASE(m_slmawc, &slock_slmawc)

	m_slmawc->memManager->freeAll();

	free(m_slmawc);
}

TEST(spin_locks_are_applied_sequentially, then_sleep_count_should_be_0)
{
    TEST_ASSERT_NOT_NULL(m_slmawc);

	TEST_ASSERT_EQUAL_UINT32(sleeps_count_slmawc, 0);
}

TEST_GROUP_RUNNER(spin_locks_are_applied_sequentially)
{
    RUN_TEST_CASE(spin_locks_are_applied_sequentially, then_sleep_count_should_be_0);
}


