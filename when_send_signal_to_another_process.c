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
#include "signalmanager.h"

TEST_GROUP(send_signal_to_another_process);

#define SIZE_SSTAP (1024 * 16)

ISignalManager     sm_sstap;
IProcessManager    pm_sstap;
IThreadHelper      th_sstap;
ISharedMemManager  smm_sstap;

SharMemHeader      shar_mem_hdr_sstap; 
SharMemHeader      shar_mem_hdr_sstap_1; 

char*              pm_args_sstap[4];

TEvent             event_sstap;
TProcess           proc_sstap;

int                signal_interrupt_func_calls_sstap = 0;

void create_signal_manager_sstap()
{
    sm_sstap = (ISignalManager)malloc(sizeof(SISignalManager));
	sm_sstap->errorLogger           = &sFakeErrorLogger;
	sm_sstap->signalCtor            = signalCtor;
	sm_sstap->dispatchQueuedSignals = dispatchQueuedSignals;
	sm_sstap->signalDtor            = signalDtor;
	sm_sstap->setSignal             = setSignal;
    sm_sstap->queueSignal           = queueSignal;
	sm_sstap->sentSignal            = sentSignal;
}

void create_shar_mem_manager_sstap()
{
    smm_sstap = (ISharedMemManager)malloc(sizeof(SISharedMemManager));
	smm_sstap->errorLogger          = &sFakeErrorLogger;
	smm_sstap->memManager           = &sFakeMemManager;
	smm_sstap->spinLockMan          = spinLockManager;
    smm_sstap->sharMemCreate        = sharMemCreate;
    smm_sstap->allocSharedMem       = allocSharedMem;
	smm_sstap->openSharedMemSegment = openSharedMemSegment;
    smm_sstap->sizeMultiply         = sizeMultiply;
    smm_sstap->addSize              = addSize;
	smm_sstap->deleteSharedMemory   = deleteSharedMemory;
    smm_sstap->sharMemCtor          = sharMemCtor;
}

void create_thread_helper_sstap()
{
    th_sstap = (IThreadHelper)malloc(sizeof(SIThreadHelper));
	th_sstap->errorLogger           = &sFakeErrorLogger;
	th_sstap->threadHelpCtor        = threadHelpCtor;
	th_sstap->startThread           = startThread;
	th_sstap->waitForEvent          = waitForEvent;
    th_sstap->waitForMultipleEvents = waitForMultipleEvents;
}

void signal_interrupt_func_sstap  () 
{ 
	HANDLE currentThread = GetCurrentThread(); 
	signal_interrupt_func_calls_sstap++; 
    
	TerminateThread(currentThread, 0); 
}

SETUP_DEPENDENCIES(send_signal_to_another_process) 
{
	create_signal_manager_sstap();
	create_shar_mem_manager_sstap();
    create_thread_helper_sstap();

	pm_sstap = (IProcessManager)malloc(sizeof(SIProcessManager));
	pm_sstap->errorLogger         = &sFakeErrorLogger;
	pm_sstap->startSubProcess     = startSubProcess;
	pm_sstap->killAllSubProcesses = killAllSubProcesses;
}

void proc_func_sstap()
{
	void*       procIdMem;
	int*        procIdPointer;
	int         procId;
    HANDLE      notifyEvent = OpenEvent(
		                     EVENT_ALL_ACCESS, 
						     False, 
						     TEXT("Global\\NotifyEvent"));   

    if (notifyEvent == NULL)
        return;
    
	create_signal_manager_sstap();
	create_shar_mem_manager_sstap();
	create_thread_helper_sstap();

	shar_mem_hdr_sstap_1  = smm_sstap->openSharedMemSegment(
		                                     smm_sstap, 
											 NULL, 
											 True, 
    										 SIZE_SSTAP);
    
	procIdMem     = (void*)((char*)shar_mem_hdr_sstap_1 
		                        + shar_mem_hdr_sstap_1->freeoffset
						        - ALIGN_DEFAULT(sizeof(int)));
    
	procIdPointer = (int*)procIdMem;
    procId        = *procIdPointer;

	SetEvent(notifyEvent);

	th_sstap->waitForEvent(th_sstap, notifyEvent);

    sm_sstap->sentSignal(sm_sstap, procId, SIGNAL_INTERRUPT);

	SetEvent(notifyEvent);
}

GIVEN(send_signal_to_another_process) 
{
	void*         procIdMem;
	int*          procId; 

	pm_args_sstap[0] = "---";    
    pm_args_sstap[1] = "func";
    pm_args_sstap[2] = NULL;
    pm_args_sstap[3] = "proc_func_sstap";

	shar_mem_hdr_sstap = smm_sstap->sharMemCreate(smm_sstap, SIZE_SSTAP); 
	smm_sstap->sharMemCtor(smm_sstap);

    procIdMem  = smm_sstap->allocSharedMem(
		                    smm_sstap, 
						    sizeof(int));

    procId     = (int*)procIdMem;
	*procId    = GetCurrentProcessId();

    sm_sstap->signalCtor(sm_sstap);
	sm_sstap->setSignal(sm_sstap, SIGNAL_INTERRUPT, signal_interrupt_func_sstap);

	Sleep(1000);

    event_sstap = CreateEvent(
		              NULL, 
					  False, 
					  False, 
					  TEXT("Global\\NotifyEvent"));

    TEST_ASSERT_NOT_NULL(event_sstap);

	proc_sstap = pm_sstap->startSubProcess(pm_sstap, 0, pm_args_sstap);

	TEST_ASSERT_NOT_NULL(proc_sstap);

	th_sstap->waitForEvent(th_sstap, event_sstap);
}

WHEN(send_signal_to_another_process)
{
    SetEvent(event_sstap);

	th_sstap->waitForEvent(th_sstap, event_sstap);

	Sleep(2000);
}

TEST_TEAR_DOWN(send_signal_to_another_process)
{
	pm_sstap->killAllSubProcesses();

	smm_sstap->memManager->freeAll();

	free(sm_sstap);
    free(pm_sstap);

	free(smm_sstap);
	free(th_sstap);
}

TEST(send_signal_to_another_process, then_interrupt_signal_should_be_queued)
{
    TEST_ASSERT_EQUAL_INT(signalQueue, 2);
    TEST_ASSERT_EQUAL_INT(signalMask, 0);  
}

TEST_GROUP_RUNNER(send_signal_to_another_process)
{
    RUN_TEST_CASE(send_signal_to_another_process, then_interrupt_signal_should_be_queued);
}


