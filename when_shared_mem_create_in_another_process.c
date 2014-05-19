#include "unity_fixture.h"
#include "fakeerrorlogger.h"
#include "sharedmemmanager.h"
#include "fakememmanager.h"
#include <windows.h>
#include "sharedmem.h"
#include "spinlockmanager.h"

TEST_GROUP(shared_mem_create_in_another_process);

ISharedMemManager smm_smciap;
SharMemHeader     shar_mem_hdr_smciap; 
size_t            size_smciap;
TSharMemHandler   existed_segm_smciap; 

void proc_func_smciap()
{

}

SETUP_DEPENDENCIES(shared_mem_create_in_another_process) 
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

GIVEN(shared_mem_create_in_another_process) 
{
    size_smciap = 1024 * 16;
}

WHEN(shared_mem_create_in_another_process)
{
    shar_mem_hdr_smciap = smm_smciap->sharMemCreate(smm_smciap, size_smciap); 
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


