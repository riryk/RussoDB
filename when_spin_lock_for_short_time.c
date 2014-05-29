#include "unity_fixture.h"
#include "fakeerrorlogger.h"
#include "spinlockmanager.h"
#include "fakememmanager.h"
#include <windows.h>
#include "spin.h"
#include "threadhelper.h"
#include "spinlockstrategy.h"

TEST_GROUP(spin_lock_for_short_time);

ISpinLockManager   m_slfst;
IThreadHelper      th_slfst;
TSpinLock          slock_slfst;
int                sleeps_count_slfst;
TThreadId          thread_id_slfst;
int                thread_2_sleep_time_slfst;
int                sleeps_slfst[1000];       
int                sleeps_count_slfst = 0;
TThread            threadHandle_slfst;

void sleepAndTrack(int sleepMilliseconds)
{
	sleeps_slfst[sleeps_count_slfst++] = sleepMilliseconds;
	Sleep(sleepMilliseconds);
}

SETUP_DEPENDENCIES(spin_lock_for_short_time) 
{
    m_slfst = (ISpinLockManager)malloc(sizeof(SISpinLockManager));
    m_slfst->errorLogger     = &sFakeErrorLogger;
	m_slfst->memManager      = &sFakeMemManager;
	m_slfst->slockStrategy   = &sSpinLockStrategy;
	m_slfst->spinLockCtor    = &spinLockCtor;
    m_slfst->spinLockAcquire = spinLockAcquire;
    m_slfst->spinLockRelease = spinLockRelease;
	m_slfst->spinLockInit    = spinLockInit;

	th_slfst = (IThreadHelper)malloc(sizeof(SIThreadHelper));
	th_slfst->threadHelpCtor = &threadHelpCtor;
    th_slfst->errorLogger    = &sFakeErrorLogger;
    th_slfst->startThread    = startThread;
}

#ifdef _WIN32

DWORD WINAPI spinLockFunc_slfst(LPVOID lpParam) 
{
	SPIN_LOCK_ACQUIRE(m_slfst, &slock_slfst);

	Sleep(thread_2_sleep_time_slfst * 1000L);

	SPIN_LOCK_RELEASE(m_slfst, &slock_slfst);
}

#endif

GIVEN(spin_lock_for_short_time) 
{
    slock_slfst               = 0;
	thread_2_sleep_time_slfst = 20;

	m_slfst->spinLockCtor(m_slfst, sleepAndTrack);

	threadHandle_slfst 
		= th_slfst->startThread(
		     th_slfst, 
			 spinLockFunc_slfst, 
			 NULL, 
			 thread_id_slfst);

	/* We need to sleep for 2 seconds 
	 * and wait until the thread launchs and  
	 * acquires the lock.
	 */
    Sleep(2 * 1000L);
}

WHEN(spin_lock_for_short_time)
{
    sleeps_count_slfst = SPIN_LOCK_ACQUIRE(m_slfst, &slock_slfst);
}

TEST_TEAR_DOWN(spin_lock_for_short_time)
{
	SPIN_LOCK_RELEASE(m_slfst, &slock_slfst)

#ifdef _WIN32
    TerminateThread(threadHandle_slfst, 0); 
    CloseHandle(threadHandle_slfst);
#endif
    
	m_slfst->memManager->freeAll();
     
	free(m_slfst);
	free(th_slfst);
}

TEST(spin_lock_for_short_time, then_total_sleep_time_must_be_enough)
{
	int  i;
	int  sleeps_time_total = 0;
	int  total_spin_ticks  = sleeps_count_slfst * SPINS_DEFAULT_NUM;

	TEST_ASSERT_TRUE(sleeps_count_slfst > 0);

    for (i = 0; i < sleeps_count_slfst; i++)
	{
        sleeps_time_total += sleeps_slfst[i];       
	}

	TEST_ASSERT_TRUE(sleeps_time_total >= thread_2_sleep_time_slfst * 1000L)
}

TEST(spin_lock_for_short_time, then_total_sleep_times_must_be_in_increasing_order)
{
	int i;
	int prev, curr;

	if (sleeps_count_slfst == 1)
		return;

    for (i = 1; i < sleeps_count_slfst; i++)
	{
        prev = sleeps_slfst[i - 1];
		curr = sleeps_slfst[i];

        TEST_ASSERT_TRUE(curr >= (int)(prev * 1.5));
	}
}

TEST_GROUP_RUNNER(spin_lock_for_short_time)
{
    RUN_TEST_CASE(spin_lock_for_short_time, then_total_sleep_time_must_be_enough);
    RUN_TEST_CASE(spin_lock_for_short_time, then_total_sleep_times_must_be_in_increasing_order);
}


