#include "unity_fixture.h"
#include "fakeerrorlogger.h"
#include "sharedmemmanager.h"
#include "fakememmanager.h"
#include <windows.h>
#include "sharedmem.h"
#include "spinlockmanager.h"
#include "process_functions.h"
#include "processhelper.h"
#include "threadhelper.h"

TEST_GROUP(shared_mem_create_in_another_process);

ISharedMemManager smm_smciap;
IThreadHelper     th_smciap;
SharMemHeader     shar_mem_hdr_smciap; 
SharMemHeader     shar_mem_hdr_smciap_1; 
size_t            size_smciap;
TSharMemHandler   existed_segm_smciap; 
IProcessManager   pm_smciap;
char*             pm_args_smciap[4];
TProcess          proc_smciap;
TEvent            event_smciap;

void create_shar_mem_manager_smciap()
{
    smm_smciap = (ISharedMemManager)malloc(sizeof(SISharedMemManager));
	smm_smciap->errorLogger          = &sFakeErrorLogger;
	smm_smciap->memManager           = &sFakeMemManager;
    smm_smciap->sharMemCreate        = sharMemCreate;
    smm_smciap->initSharMemAccess    = initSharMemAccess;
    smm_smciap->allocSharedMem       = allocSharedMem;
	smm_smciap->openSharedMemSegment = openSharedMemSegment;
    smm_smciap->sizeMultiply         = sizeMultiply;
    smm_smciap->addSize              = addSize;
	smm_smciap->deleteSharedMemory   = deleteSharedMemory;
}

#ifdef _WIN32

void proc_func_smciap()
{
	size_t        freeOffset;
	void*         memToSet;
    HANDLE        notifyEvent = OpenEvent(
		                          EVENT_ALL_ACCESS, 
								  False, 
								  TEXT("Global\\NotifyEvent"));

	if (notifyEvent == NULL)
        return;

	create_shar_mem_manager_smciap();

	size_smciap         = 1024 * 16;
    shar_mem_hdr_smciap = smm_smciap->sharMemCreate(smm_smciap, size_smciap); 

	freeOffset = shar_mem_hdr_smciap->freeoffset;
    memToSet   = shar_mem_hdr_smciap + freeOffset;

	memset(memToSet, 7, 1000); 
	SetEvent(notifyEvent);
	Sleep(1000000);
}

#endif

SETUP_DEPENDENCIES(shared_mem_create_in_another_process) 
{
    create_shar_mem_manager_smciap();

	pm_smciap = (IProcessManager)malloc(sizeof(SIProcessManager));
	pm_smciap->errorLogger         = &sFakeErrorLogger;
	pm_smciap->startSubProcess     = startSubProcess;
	pm_smciap->killAllSubProcesses = killAllSubProcesses;

    th_smciap = (IThreadHelper)malloc(sizeof(SIThreadHelper));
    th_smciap->errorLogger         = &sFakeErrorLogger;
    th_smciap->threadHelpCtor      = threadHelpCtor;
    th_smciap->startThread         = startThread;
    th_smciap->spinWait            = spinWait;
    th_smciap->waitForEvent        = waitForEvent;
}

GIVEN(shared_mem_create_in_another_process) 
{
    size_smciap = 1024 * 16;

    pm_args_smciap[0] = "---";
    pm_args_smciap[1] = "func";
    pm_args_smciap[2] = NULL;
    pm_args_smciap[3] = "proc_func_smciap";

	event_smciap = CreateEvent(
		              NULL, 
					  False, 
					  False, 
					  TEXT("Global\\NotifyEvent"));

    TEST_ASSERT_NOT_NULL(event_smciap);
}

WHEN(shared_mem_create_in_another_process)
{
	proc_smciap = pm_smciap->startSubProcess(pm_smciap, 0, pm_args_smciap);

	TEST_ASSERT_NOT_NULL(event_smciap);

	th_smciap->waitForEvent(th_smciap, event_smciap);

	shar_mem_hdr_smciap_1 = smm_smciap->openSharedMemSegment(smm_smciap, NULL, True, size_smciap);
}

TEST_TEAR_DOWN(shared_mem_create_in_another_process)
{
	smm_smciap->deleteSharedMemory(
	      smm_smciap, 
		  (void*)shar_mem_hdr_smciap,  
		  shar_mem_hdr_smciap->handle);

	smm_smciap->memManager->freeAll();

	free(smm_smciap);
}

TEST(shared_mem_create_in_another_process, then_test)
{
}

TEST_GROUP_RUNNER(shared_mem_create_in_another_process)
{
    RUN_TEST_CASE(shared_mem_create_in_another_process, then_test);
}


