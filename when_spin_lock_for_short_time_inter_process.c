#include "unity_fixture.h"
#include "fakeerrorlogger.h"
#include "spinlockmanager.h"
#include "fakememmanager.h"
#include <windows.h>
#include "spin.h"
#include "threadhelper.h"

TEST_GROUP(spin_lock_for_short_time_inter_process);

ISharedMemManager  smm_slfstip;
ISpinLockManager   m_slfstip;
IProcessManager    pm_slfstip;
TSpinLock          slock_slfstip;

SharMemHeader      shar_mem_hdr_slfstip; 
SharMemHeader      shar_mem_hdr_slfstip_1; 

char*              pm_args_slfstip[4];
int                sleeps_count_slfstip;
TThreadId          thread_id_slfstip;
int                thread_2_sleep_time_slfstip;
int                sleeps_slfstip[1000];       
int                sleeps_count_slfstip = 0;
TThread            threadHandle_slfstip;
TEvent             event_slfstip;

void sleep_slfstip(int sleepMilliseconds)
{
	sleeps_slfstip[sleeps_count_slfstip++] = sleepMilliseconds;
	Sleep(sleepMilliseconds);
}

void create_spin_lock_manager_slfstip()
{
    m_slfstip = (ISpinLockManager)malloc(sizeof(SISpinLockManager));
    m_slfstip->errorLogger     = &sFakeErrorLogger;
	m_slfstip->memManager      = &sFakeMemManager;
	m_slfstip->spinLockCtor    = &spinLockCtor;
    m_slfstip->spinLockAcquire = spinLockAcquire;
    m_slfstip->spinLockRelease = spinLockRelease;
}

void create_shar_mem_manager_slfstip()
{
    smm_slfstip = (ISharedMemManager)malloc(sizeof(SISharedMemManager));
	smm_slfstip->errorLogger          = &sFakeErrorLogger;
	smm_slfstip->memManager           = &sFakeMemManager;
	smm_slfstip->spinLockMan          = spinLockManager;
    smm_slfstip->sharMemCreate        = sharMemCreate;
    smm_slfstip->allocSharedMem       = allocSharedMem;
	smm_slfstip->openSharedMemSegment = openSharedMemSegment;
    smm_slfstip->sizeMultiply         = sizeMultiply;
    smm_slfstip->addSize              = addSize;
	smm_slfstip->deleteSharedMemory   = deleteSharedMemory;
    smm_slfstip->sharMemCtor          = sharMemCtor;
}

SETUP_DEPENDENCIES(spin_lock_for_short_time_inter_process) 
{
	create_spin_lock_manager_slfstip();
    create_shar_mem_manager_slfstip();

	pm_slfstip = (IProcessManager)malloc(sizeof(SIProcessManager));
	pm_slfstip->errorLogger         = &sFakeErrorLogger;
	pm_slfstip->startSubProcess     = startSubProcess;
	pm_slfstip->killAllSubProcesses = killAllSubProcesses;

	th_slfstip = (IThreadHelper)malloc(sizeof(SIThreadHelper));
	th_slfstip->threadHelpCtor = threadHelpCtor;
    th_slfstip->errorLogger    = &sFakeErrorLogger;
    th_slfstip->startThread    = startThread;
    th_slfstip->spinWait       = spinWait;
    th_slfstip->waitForEvent   = waitForEvent;
}

DWORD WINAPI spinLockFunc_slfst(LPVOID lpParam) 
{
	SPIN_LOCK_ACQUIRE(m_slfst, &slock_slfst);

	Sleep(thread_2_sleep_time_slfst * 1000L);

	SPIN_LOCK_RELEASE(m_slfst, &slock_slfst);
}

void proc_func_slfstip()
{
    HANDLE   notifyEvent = OpenEvent(
		                     EVENT_ALL_ACCESS, 
						     False, 
						     TEXT("Global\\NotifyEvent"));   

    if (notifyEvent == NULL)
        return;
    
	create_spin_lock_manager_slfstip();

	shar_mem_hdr_slfstip_1 = smm_slfstip->openSharedMemSegment(
		                                     smm_slfstip, 
											 NULL, 
											 True, 
    										 size_slfstip);


    SPIN_LOCK_ACQUIRE(m_slfstip, &slock_slfstip);

    SetEvent(notifyEvent);

	Sleep(3 * 1000L);

	SPIN_LOCK_RELEASE(m_slfstip, &slock_slfstip);
}

GIVEN(spin_lock_for_short_time_inter_process) 
{
	shar_mem_hdr_slfstip = smm_slfstip->sharMemCreate(smm_slfstip, size_slfstip); 
	smm_slfstip->sharMemCtor(smm_slfstip);
	slock_slfstip       = (TSpinLock)smm_slfstip->allocSharedMem(
		                                 smm_slfstip, 
										 sizeof(SSharMemTest));
    slock_slfstip       = 0;
    
	pm_args_slfstip[0] = "---";
    pm_args_slfstip[1] = "func";
    pm_args_slfstip[2] = NULL;
    pm_args_slfstip[3] = "proc_func_slfstip";

	m_slfstip->spinLockCtor(m_slfstip, sleep_slfstip);

    event_slfstip = CreateEvent(
		              NULL, 
					  False, 
					  False, 
					  TEXT("Global\\NotifyEvent"));

    TEST_ASSERT_NOT_NULL(event_slfstip);

	proc_slfstip = pm_slfstip->startSubProcess(pm_slfstip, 0, pm_args_slfstip);

	TEST_ASSERT_NOT_NULL(proc_slfstip);

	th_slfstip->waitForEvent(th_slfstip, event_slfstip);
}

WHEN(spin_lock_for_short_time_inter_process)
{
    sleeps_count_slfst = SPIN_LOCK_ACQUIRE(m_slfst, &slock_slfst);
}

TEST_TEAR_DOWN(spin_lock_for_short_time_inter_process)
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

TEST(spin_lock_for_short_time_inter_process, then_total_sleep_time_must_be_enough)
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

TEST(spin_lock_for_short_time_inter_process, then_total_sleep_times_must_be_in_increasing_order)
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

TEST_GROUP_RUNNER(spin_lock_for_short_time_inter_process)
{
    RUN_TEST_CASE(spin_lock_for_short_time_inter_process, then_total_sleep_time_must_be_enough);
    RUN_TEST_CASE(spin_lock_for_short_time_inter_process, then_total_sleep_times_must_be_in_increasing_order);
}


