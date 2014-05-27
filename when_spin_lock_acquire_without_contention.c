#include "unity_fixture.h"
#include "fakeerrorlogger.h"
#include "spinlockmanager.h"
#include "fakememmanager.h"
#include <windows.h>
#include "spin.h"
#include "spinlockstrategy.h"

TEST_GROUP(spin_lock_manager_acquire_without_contention);

ISpinLockManager  m_slmawc;
TSpinLock         slock_slmawc;
int               sleeps_count_slmawc;

SETUP_DEPENDENCIES(spin_lock_manager_acquire_without_contention) 
{
    m_slmawc = (ISpinLockManager)malloc(sizeof(SISpinLockManager));
    m_slmawc->errorLogger     = &sFakeErrorLogger;
	m_slmawc->memManager      = &sFakeMemManager;
	m_slmawc->slockStrategy   = &sSpinLockStrategy;
    m_slmawc->spinLockAcquire = spinLockAcquire;
    m_slmawc->spinLockRelease = spinLockRelease;
}

GIVEN(spin_lock_manager_acquire_without_contention) 
{
    slock_slmawc = 0;
}

WHEN(spin_lock_manager_acquire_without_contention)
{
    sleeps_count_slmawc = SPIN_LOCK_ACQUIRE(m_slmawc, &slock_slmawc);
}

TEST_TEAR_DOWN(spin_lock_manager_acquire_without_contention)
{
	SPIN_LOCK_RELEASE(m_slmawc, &slock_slmawc)

	m_slmawc->memManager->freeAll();

	free(m_slmawc);
}

TEST(spin_lock_manager_acquire_without_contention, then_sleep_count_should_be_0)
{
    TEST_ASSERT_NOT_NULL(m_slmawc);

	TEST_ASSERT_EQUAL_UINT32(sleeps_count_slmawc, 0);
}

TEST_GROUP_RUNNER(spin_lock_manager_acquire_without_contention)
{
    RUN_TEST_CASE(spin_lock_manager_acquire_without_contention, then_sleep_count_should_be_0);
}


