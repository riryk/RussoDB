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
#include "sharedmem_helper.h"

TEST_GROUP(shared_mem_allocate_in_another_process);

ISharedMemManager smm_smaiap;
IThreadHelper     th_smaiap;
SharMemHeader     shar_mem_hdr_smaiap; 
SharMemHeader     shar_mem_hdr_smaiap_1; 
size_t            size_smaiap;
TSharMemHandler   existed_segm_smaiap; 
IProcessManager   pm_smaiap;
char*             pm_args_smaiap[4];
TProcess          proc_smaiap;
TEvent            event_smaiap;

void create_shar_mem_manager_smaiap()
{
    smm_smaiap = (ISharedMemManager)malloc(sizeof(SISharedMemManager));
	smm_smaiap->errorLogger          = &sFakeErrorLogger;
	smm_smaiap->memManager           = &sFakeMemManager;
	smm_smaiap->spinLockMan          = spinLockManager;
    smm_smaiap->sharMemCreate        = sharMemCreate;
    smm_smaiap->allocSharedMem       = allocSharedMem;
	smm_smaiap->openSharedMemSegment = openSharedMemSegment;
    smm_smaiap->sizeMultiply         = sizeMultiply;
    smm_smaiap->addSize              = addSize;
	smm_smaiap->deleteSharedMemory   = deleteSharedMemory;
    smm_smaiap->sharMemCtor          = sharMemCtor;
}

#ifdef _WIN32

void proc_func_smaiap()
{
	size_t        freeOffset;
	void*         memToSet;
	SharMemTest   testObj; 
	char*         memToSetChar;
    HANDLE        notifyEvent = OpenEvent(
		                          EVENT_ALL_ACCESS, 
								  False, 
								  TEXT("Global\\NotifyEvent"));

	if (notifyEvent == NULL)
        return;

	create_shar_mem_manager_smaiap();

	size_smaiap         = 1024 * 16;

    shar_mem_hdr_smaiap = smm_smaiap->sharMemCreate(smm_smaiap, size_smaiap); 
	smm_smaiap->sharMemCtor(smm_smaiap);
	testObj             = (SharMemTest)smm_smaiap->allocSharedMem(
		                                 smm_smaiap, 
										 sizeof(SSharMemTest));

	testObj->field1 = 123;
    testObj->field2 = 334;
	testObj->field3 = 434;

	SetEvent(notifyEvent);
	Sleep(1000000);
}

#endif

SETUP_DEPENDENCIES(shared_mem_allocate_in_another_process) 
{
    create_shar_mem_manager_smaiap();

	pm_smaiap = (IProcessManager)malloc(sizeof(SIProcessManager));
	pm_smaiap->errorLogger         = &sFakeErrorLogger;
	pm_smaiap->startSubProcess     = startSubProcess;
	pm_smaiap->killAllSubProcesses = killAllSubProcesses;

    th_smaiap = (IThreadHelper)malloc(sizeof(SIThreadHelper));
    th_smaiap->errorLogger         = &sFakeErrorLogger;
    th_smaiap->threadHelpCtor      = threadHelpCtor;
    th_smaiap->startThread         = startThread;
    th_smaiap->spinWait            = spinWait;
    th_smaiap->waitForEvent        = waitForEvent;
}

GIVEN(shared_mem_allocate_in_another_process) 
{
    size_smaiap = 1024 * 16;

    pm_args_smaiap[0] = "---";
    pm_args_smaiap[1] = "func";
    pm_args_smaiap[2] = NULL;
    pm_args_smaiap[3] = "proc_func_smaiap";

	event_smaiap = CreateEvent(
		              NULL, 
					  False, 
					  False, 
					  TEXT("Global\\NotifyEvent"));

    TEST_ASSERT_NOT_NULL(event_smaiap);
}

WHEN(shared_mem_allocate_in_another_process)
{
	proc_smaiap = pm_smaiap->startSubProcess(pm_smaiap, 0, pm_args_smaiap);

	TEST_ASSERT_NOT_NULL(event_smaiap);

	th_smaiap->waitForEvent(th_smaiap, event_smaiap);

	shar_mem_hdr_smaiap_1 = smm_smaiap->openSharedMemSegment(smm_smaiap, NULL, True, size_smaiap);
}

TEST_TEAR_DOWN(shared_mem_allocate_in_another_process)
{
	pm_smaiap->killAllSubProcesses();

	smm_smaiap->memManager->freeAll();
    
	free(smm_smaiap);
}

TEST(shared_mem_allocate_in_another_process, then_the_object_should_be_read)
{
	void*       mem     = (void*)((char*)shar_mem_hdr_smaiap_1 
		                        + shar_mem_hdr_smaiap_1->freeoffset
						        - ALIGN_DEFAULT(sizeof(SSharMemTest)));

    SharMemTest testObj = (SharMemTest)mem; 

	TEST_ASSERT_EQUAL_UINT32(testObj->field1, 123);
	TEST_ASSERT_EQUAL_UINT32(testObj->field2, 334);
	TEST_ASSERT_EQUAL_UINT32(testObj->field3, 434);
}

TEST_GROUP_RUNNER(shared_mem_allocate_in_another_process)
{
    RUN_TEST_CASE(shared_mem_allocate_in_another_process, then_the_object_should_be_read);
}


