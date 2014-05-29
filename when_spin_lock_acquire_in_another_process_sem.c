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
#include "spinlocksemaphorestrategy.h"

TEST_GROUP(spin_lock_acquire_in_another_process_sem);

#define SIZE_SLAIAPS (1024 * 16)

ISharedMemManager  smm_slaiaps;
ISpinLockManager   m_slaiaps;
IProcessManager    pm_slaiaps;
TSpinLock*         slock_slaiaps;
IThreadHelper      th_slaiaps;

SharMemHeader      shar_mem_hdr_slaiaps; 
SharMemHeader      shar_mem_hdr_slaiaps_1; 

char*              pm_args_slaiaps[4];
int                sleeps_count_slaiaps;
TThreadId          thread_id_slaiaps;
int                thread_2_sleep_time_slaiaps;
int                sleeps_slaiaps[1000];       
int                sleeps_count_slaiaps = 0;
TThread            threadHandle_slaiaps;
TEvent             event_slaiaps;
TProcess           proc_slaiaps;

int                aux_proc_slp_slaiaps;
int                aux_proc_slaiaps;

void sleep_slaiaps(int sleepMilliseconds)
{
	sleeps_slaiaps[sleeps_count_slaiaps++] = sleepMilliseconds;
	Sleep(sleepMilliseconds);
}

void create_spin_lock_manager_slaiaps()
{
    m_slaiaps = (ISpinLockManager)malloc(sizeof(SISpinLockManager));
    m_slaiaps->errorLogger     = &sFakeErrorLogger;
	m_slaiaps->memManager      = &sFakeMemManager;
	m_slaiaps->slockStrategy   = &sSpinLockSemaphoreStrategy;
	m_slaiaps->spinLockCtor    = &spinLockCtor;
    m_slaiaps->spinLockAcquire = spinLockAcquire;
    m_slaiaps->spinLockRelease = spinLockRelease;
	m_slaiaps->spinLockInit    = spinLockInit;
}

void create_shar_mem_manager_slaiaps()
{
    smm_slaiaps = (ISharedMemManager)malloc(sizeof(SISharedMemManager));
	smm_slaiaps->errorLogger          = &sFakeErrorLogger;
	smm_slaiaps->memManager           = &sFakeMemManager;
	smm_slaiaps->spinLockMan          = spinLockManager;
    smm_slaiaps->sharMemCreate        = sharMemCreate;
    smm_slaiaps->allocSharedMem       = allocSharedMem;
	smm_slaiaps->openSharedMemSegment = openSharedMemSegment;
    smm_slaiaps->sizeMultiply         = sizeMultiply;
    smm_slaiaps->addSize              = addSize;
	smm_slaiaps->deleteSharedMemory   = deleteSharedMemory;
    smm_slaiaps->sharMemCtor          = sharMemCtor;
}

SETUP_DEPENDENCIES(spin_lock_acquire_in_another_process_sem) 
{
	create_spin_lock_manager_slaiaps();
    create_shar_mem_manager_slaiaps();

	pm_slaiaps = (IProcessManager)malloc(sizeof(SIProcessManager));
	pm_slaiaps->errorLogger         = &sFakeErrorLogger;
	pm_slaiaps->startSubProcess     = startSubProcess;
	pm_slaiaps->killAllSubProcesses = killAllSubProcesses;

	th_slaiaps = (IThreadHelper)malloc(sizeof(SIThreadHelper));
	th_slaiaps->threadHelpCtor = threadHelpCtor;
    th_slaiaps->errorLogger    = &sFakeErrorLogger;
    th_slaiaps->startThread    = startThread;
    th_slaiaps->spinWait       = spinWait;
    th_slaiaps->waitForEvent   = waitForEvent;
}

void proc_func_slaiaps()
{
	TSpinLock*  spinLock;
	void*       memSpinLock;
    HANDLE      notifyEvent = OpenEvent(
		                     EVENT_ALL_ACCESS, 
						     False, 
						     TEXT("Global\\NotifyEvent"));   

    if (notifyEvent == NULL)
        return;
    
	aux_proc_slp_slaiaps  = 20;

	create_spin_lock_manager_slaiaps();
    create_shar_mem_manager_slaiaps();

    m_slaiaps->spinLockCtor(m_slaiaps, sleep_slaiaps);

	shar_mem_hdr_slaiaps_1  = smm_slaiaps->openSharedMemSegment(
		                                     smm_slaiaps, 
											 NULL, 
											 True,  
    										 SIZE_SLAIAPS);
    
	memSpinLock = (void*)((char*)shar_mem_hdr_slaiaps_1 
		                        + shar_mem_hdr_slaiaps_1->freeoffset
						        - ALIGN_DEFAULT(sizeof(TSpinLock)));
    
	slock_slaiaps = (TSpinLock*)memSpinLock;

    SPIN_LOCK_ACQUIRE(m_slaiaps, slock_slaiaps);

    SetEvent(notifyEvent);

	Sleep(20 * 1000L);

	SPIN_LOCK_RELEASE(m_slaiaps, slock_slaiaps);
}

GIVEN(spin_lock_acquire_in_another_process_sem) 
{
	void*         sLockMem;

	pm_args_slaiaps[0] = "---";
    pm_args_slaiaps[1] = "func";
    pm_args_slaiaps[2] = NULL;
    pm_args_slaiaps[3] = "proc_func_slaiaps";

	shar_mem_hdr_slaiaps = smm_slaiaps->sharMemCreate(smm_slaiaps, SIZE_SLAIAPS); 
	smm_slaiaps->sharMemCtor(smm_slaiaps);

    sLockMem  = smm_slaiaps->allocSharedMem(
		                    smm_slaiaps, 
						    sizeof(TSpinLock));

    slock_slaiaps     = (TSpinLock*)sLockMem;
	*slock_slaiaps    = 0;

	m_slaiaps->spinLockCtor(m_slaiaps, sleep_slaiaps);
	m_slaiaps->spinLockInit(m_slaiaps, slock_slaiaps);

    event_slaiaps = CreateEvent(
		              NULL, 
					  False, 
					  False, 
					  TEXT("Global\\NotifyEvent"));

    TEST_ASSERT_NOT_NULL(event_slaiaps);

	proc_slaiaps = pm_slaiaps->startSubProcess(pm_slaiaps, 0, pm_args_slaiaps);

	TEST_ASSERT_NOT_NULL(proc_slaiaps);

	th_slaiaps->waitForEvent(th_slaiaps, event_slaiaps);
}

WHEN(spin_lock_acquire_in_another_process_sem)
{
    sleeps_count_slaiaps = SPIN_LOCK_ACQUIRE(m_slaiaps, slock_slaiaps);
}

TEST_TEAR_DOWN(spin_lock_acquire_in_another_process_sem)
{
	SPIN_LOCK_RELEASE(m_slaiaps, slock_slaiaps);

	pm_slaiaps->killAllSubProcesses();

	m_slaiaps->memManager->freeAll();
     
	free(m_slaiaps);
	free(th_slaiaps);
}

TEST(spin_lock_acquire_in_another_process_sem, then_test)
{
	
}

TEST_GROUP_RUNNER(spin_lock_acquire_in_another_process_sem)
{
    RUN_TEST_CASE(spin_lock_acquire_in_another_process_sem, then_test);
}


