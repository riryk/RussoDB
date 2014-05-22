#include "unity_fixture.h"
#include "fakeerrorlogger.h"
#include "sharedmemmanager.h"
#include "fakememmanager.h"
#include <windows.h>
#include "sharedmem.h"
#include "spinlockmanager.h"

TEST_GROUP(shared_mem_reattach);

ISharedMemManager smm_smr;
SharMemHeader     shar_mem_hdr_smr; 
SharMemHeader     shar_mem_hdr_reattached_smr; 
size_t            size_smr;
TSharMemHandler   existed_segm_smr; 

SETUP_DEPENDENCIES(shared_mem_reattach) 
{
    smm_smr = (ISharedMemManager)malloc(sizeof(SISharedMemManager));
	smm_smr->errorLogger          = &sFakeErrorLogger;
	smm_smr->memManager           = &sFakeMemManager;
    smm_smr->sharMemCreate        = sharMemCreate;
    smm_smr->allocSharedMem       = allocSharedMem;
	smm_smr->openSharedMemSegment = openSharedMemSegment;
    smm_smr->sizeMultiply         = sizeMultiply;
    smm_smr->addSize              = addSize;
	smm_smr->deleteSharedMemory   = deleteSharedMemory;
	smm_smr->sharedMemoryReAttach = sharedMemoryReAttach;
    smm_smr->detachSharedMemory   = detachSharedMemory;
}

GIVEN(shared_mem_reattach) 
{
    size_smr         = 1024 * 16;
    shar_mem_hdr_smr = smm_smr->sharMemCreate(smm_smr, size_smr); 
	smm_smr->detachSharedMemory(smm_smr, shar_mem_hdr_smr);
}

WHEN(shared_mem_reattach)
{
	shar_mem_hdr_reattached_smr = 
		smm_smr->sharedMemoryReAttach(
		  smm_smr, 
          shar_mem_hdr_smr,
		  shar_mem_hdr_smr->handle);
}

TEST_TEAR_DOWN(shared_mem_reattach)
{
	smm_smr->deleteSharedMemory(
	      smm_smr, 
		  (void*)shar_mem_hdr_reattached_smr,  
		  shar_mem_hdr_reattached_smr->handle);

	smm_smr->memManager->freeAll();

	free(smm_smr);
}

TEST(shared_mem_reattach, then_test)
{
    TEST_ASSERT_NOT_NULL(smm_smr);
}

TEST_GROUP_RUNNER(shared_mem_reattach)
{
    RUN_TEST_CASE(shared_mem_reattach, then_test);
}


