#include "unity_fixture.h"
#include "fakeerrorlogger.h"
#include "spinlockmanager.h"
#include "fakememmanager.h"
#include <windows.h>
#include "spin.h"
#include "spinlockstrategy.h"
#include "spinlocksemaphorestrategy.h"

TEST_GROUP(spin_lock_manager_acquire_without_contention_sem);

ISpinLockManager  m_slmawcs;
TSpinLock         slock_slmawcs;
int               sleeps_count_slmawcs;

void sleep_slmawcs(int sleepMilliseconds)
{
	Sleep(sleepMilliseconds);
}

SETUP_DEPENDENCIES(spin_lock_manager_acquire_without_contention_sem) 
{
    m_slmawcs = (ISpinLockManager)malloc(sizeof(SISpinLockManager));
    m_slmawcs->errorLogger     = &sFakeErrorLogger;
	m_slmawcs->memManager      = &sFakeMemManager;
	m_slmawcs->slockStrategy   = &sSpinLockSemaphoreStrategy;
	m_slmawcs->spinLockCtor    = spinLockCtor;
    m_slmawcs->spinLockInit    = spinLockInit;
    m_slmawcs->spinLockAcquire = spinLockAcquire;
    m_slmawcs->spinLockRelease = spinLockRelease;
}

GIVEN(spin_lock_manager_acquire_without_contention_sem) 
{
	m_slmawcs->spinLockCtor(m_slmawcs, sleep_slmawcs);
	m_slmawcs->spinLockInit(m_slmawcs, &slock_slmawcs);
}

WHEN(spin_lock_manager_acquire_without_contention_sem)
{
    sleeps_count_slmawcs = SPIN_LOCK_ACQUIRE(m_slmawcs, &slock_slmawcs);
}

TEST_TEAR_DOWN(spin_lock_manager_acquire_without_contention_sem)
{
	SPIN_LOCK_RELEASE(m_slmawcs, &slock_slmawcs)

	m_slmawcs->memManager->freeAll();

	free(m_slmawcs);
}

TEST(spin_lock_manager_acquire_without_contention_sem, then_sleep_count_should_be_0)
{
    TEST_ASSERT_NOT_NULL(m_slmawcs);

	TEST_ASSERT_EQUAL_UINT32(sleeps_count_slmawcs, 0);
}

TEST_GROUP_RUNNER(spin_lock_manager_acquire_without_contention_sem)
{
    RUN_TEST_CASE(spin_lock_manager_acquire_without_contention_sem, then_sleep_count_should_be_0);
}


