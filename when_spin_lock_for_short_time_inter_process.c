#include "unity_fixture.h"
#include "fakeerrorlogger.h"
#include "spinlockmanager.h"
#include "fakememmanager.h"
#include <windows.h>
#include "spin.h"
#include "threadhelper.h"
#include "sharedmemmanager.h"
#include "process_functions.h"
#include "processhelper.h"
#include "sharedmem.h"
#include "sharedmem_helper.h"
#include "spinlockstrategy.h"

TEST_GROUP(spin_lock_for_short_time_inter_process);

#define SIZE_SLFSTIP (1024 * 16)

ISharedMemManager  smm_slfstip;
ISpinLockManager   m_slfstip;
IProcessManager    pm_slfstip;
TSpinLock*         slock_slfstip;
IThreadHelper      th_slfstip;

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
TProcess           proc_slfstip;

int                aux_proc_slp_slfstip;
int                aux_proc_slfstip;

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
	m_slfstip->slockStrategy   = &sSpinLockStrategy;
	m_slfstip->spinLockCtor    = &spinLockCtor;
    m_slfstip->spinLockAcquire = spinLockAcquire;
    m_slfstip->spinLockRelease = spinLockRelease;
	m_slfstip->spinLockInit    = spinLockInit;
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

void proc_func_slfstip()
{
	TSpinLock*  spinLock;
	void*       memSpinLock;
    HANDLE      notifyEvent = OpenEvent(
		                     EVENT_ALL_ACCESS, 
						     False, 
						     TEXT("Global\\NotifyEvent"));   

    if (notifyEvent == NULL)
        return;
    
	aux_proc_slp_slfstip  = 20;

	create_spin_lock_manager_slfstip();
    create_shar_mem_manager_slfstip();

    m_slfstip->spinLockCtor(m_slfstip, sleep_slfstip);

	shar_mem_hdr_slfstip_1  = smm_slfstip->openSharedMemSegment(
		                                     smm_slfstip, 
											 NULL, 
											 True, 
    										 SIZE_SLFSTIP);
    
	memSpinLock = (void*)((char*)shar_mem_hdr_slfstip_1 
		                        + shar_mem_hdr_slfstip_1->freeoffset
						        - AlignDefault(sizeof(TSpinLock)));
    
	slock_slfstip = (TSpinLock*)memSpinLock;

    SPIN_LOCK_ACQUIRE(m_slfstip, slock_slfstip);

    SetEvent(notifyEvent);

	Sleep(aux_proc_slp_slfstip * 1000L);

	SPIN_LOCK_RELEASE(m_slfstip, slock_slfstip);
}

GIVEN(spin_lock_for_short_time_inter_process) 
{
	void*         sLockMem;
	TSpinLock*    sLock;

    aux_proc_slfstip      = 20;

	shar_mem_hdr_slfstip = smm_slfstip->sharMemCreate(smm_slfstip, SIZE_SLFSTIP); 
	smm_slfstip->sharMemCtor(smm_slfstip);

    sLockMem  = smm_slfstip->allocSharedMem(
		                    smm_slfstip, 
						    sizeof(TSpinLock));

    sLock     = (TSpinLock*)sLockMem;

	slock_slfstip        = sLock;
    *slock_slfstip       = 0;
    
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
    sleeps_count_slfstip = SPIN_LOCK_ACQUIRE(m_slfstip, slock_slfstip);
}

TEST_TEAR_DOWN(spin_lock_for_short_time_inter_process)
{
	SPIN_LOCK_RELEASE(m_slfstip, slock_slfstip);

	pm_slfstip->killAllSubProcesses();

	m_slfstip->memManager->freeAll();
     
	free(m_slfstip);
	free(th_slfstip);
}

TEST(spin_lock_for_short_time_inter_process, then_total_sleep_time_must_be_enough)
{
	int  i;
	int  sleeps_time_total = 0;
	int  total_spin_ticks  = sleeps_count_slfstip * SPINS_DEFAULT_NUM;

	TEST_ASSERT_TRUE(sleeps_count_slfstip > 0);

    for (i = 0; i < sleeps_count_slfstip; i++)
	{
        sleeps_time_total += sleeps_slfstip[i];       
	}

	TEST_ASSERT_TRUE(sleeps_time_total >= aux_proc_slfstip * 1000L)
}

TEST(spin_lock_for_short_time_inter_process, then_total_sleep_times_must_be_in_increasing_order)
{
	int i;
	int prev, curr;

	if (sleeps_count_slfstip == 1)
		return;

    for (i = 1; i < sleeps_count_slfstip; i++)
	{
        prev = sleeps_slfstip[i - 1];
		curr = sleeps_slfstip[i];

        TEST_ASSERT_TRUE(curr >= (int)(prev * 1.5));
	}
}

TEST_GROUP_RUNNER(spin_lock_for_short_time_inter_process)
{
    RUN_TEST_CASE(spin_lock_for_short_time_inter_process, then_total_sleep_time_must_be_enough);
    RUN_TEST_CASE(spin_lock_for_short_time_inter_process, then_total_sleep_times_must_be_in_increasing_order);
}


