#include "unity_fixture.h"
#include "fakeerrorlogger.h"
#include "sharedmemmanager.h"
#include "fakememmanager.h"
#include <windows.h>
#include "sharedmem.h"
#include "spinlockmanager.h"

TEST_GROUP(shared_mem_create);

ISharedMemManager smm_smc;
SharMemHeader     shar_mem_hdr_smc; 
size_t            size_smc;
TSharMemHandler   existed_segm_smc; 

SETUP_DEPENDENCIES(shared_mem_create) 
{
    smm_smc = (ISharedMemManager)malloc(sizeof(SISharedMemManager));
	smm_smc->errorLogger          = &sFakeErrorLogger;
	smm_smc->memManager           = &sFakeMemManager;
    smm_smc->sharMemCreate        = sharMemCreate;
    smm_smc->initSharMemAccess    = initSharMemAccess;
    smm_smc->allocSharedMem       = allocSharedMem;
	smm_smc->openSharedMemSegment = openSharedMemSegment;
    smm_smc->sizeMultiply         = sizeMultiply;
    smm_smc->addSize              = addSize;
	smm_smc->deleteSharedMemory   = deleteSharedMemory;
}

GIVEN(shared_mem_create) 
{
    size_smc = 1024 * 16;
}

WHEN(shared_mem_create)
{
    shar_mem_hdr_smc = smm_smc->sharMemCreate(smm_smc, size_smc); 
}

TEST_TEAR_DOWN(shared_mem_create)
{
	smm_smc->deleteSharedMemory(
	      smm_smc, 
		  (void*)shar_mem_hdr_smc,  
		  shar_mem_hdr_smc->handle);

	smm_smc->memManager->freeAll();

	free(smm_smc);
}

TEST(shared_mem_create, then_shar_mem_should_not_be_null)
{
    TEST_ASSERT_NOT_NULL(smm_smc);
    TEST_ASSERT_NOT_NULL(shar_mem_hdr_smc);
}

TEST(shared_mem_create, then_we_should_be_able_to_open_this_segment)
{
    existed_segm_smc = smm_smc->openSharedMemSegment(smm_smc, NULL, False, size_smc);
	TEST_ASSERT_NOT_NULL(existed_segm_smc);
}

TEST_GROUP_RUNNER(shared_mem_create)
{
    RUN_TEST_CASE(shared_mem_create, then_shar_mem_should_not_be_null);
    RUN_TEST_CASE(shared_mem_create, then_we_should_be_able_to_open_this_segment);
}


