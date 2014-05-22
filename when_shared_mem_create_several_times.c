#include "unity_fixture.h"
#include "fakeerrorlogger.h"
#include "sharedmemmanager.h"
#include "fakememmanager.h"
#include <windows.h>
#include "sharedmem.h"
#include "spinlockmanager.h"

TEST_GROUP(shared_mem_create_several_times);

ISharedMemManager smm_smcst;
SharMemHeader     shar_mem_hdr_smcst1; 
SharMemHeader     shar_mem_hdr_smcst2; 
SharMemHeader     shar_mem_hdr_smcst3; 
size_t            size_smcst;
TSharMemHandler   existed_segm_smcst; 

SETUP_DEPENDENCIES(shared_mem_create_several_times) 
{
    smm_smcst = (ISharedMemManager)malloc(sizeof(SISharedMemManager));
	smm_smcst->errorLogger          = &sFakeErrorLogger;
	smm_smcst->memManager           = &sFakeMemManager;
    smm_smcst->sharMemCreate        = sharMemCreate;
    smm_smcst->allocSharedMem       = allocSharedMem;
	smm_smcst->openSharedMemSegment = openSharedMemSegment;
    smm_smcst->sizeMultiply         = sizeMultiply;
    smm_smcst->addSize              = addSize;
	smm_smcst->deleteSharedMemory   = deleteSharedMemory;
}

GIVEN(shared_mem_create_several_times) 
{
    size_smcst = 1024 * 16;
}

WHEN(shared_mem_create_several_times)
{
    shar_mem_hdr_smcst1 = smm_smcst->sharMemCreate(smm_smcst, size_smcst); 
    shar_mem_hdr_smcst2 = smm_smcst->sharMemCreate(smm_smcst, size_smcst); 
	shar_mem_hdr_smcst3 = smm_smcst->sharMemCreate(smm_smcst, size_smcst); 
}

TEST_TEAR_DOWN(shared_mem_create_several_times)
{
	errorMessages = 0;

	smm_smcst->deleteSharedMemory(
	      smm_smcst, 
		  (void*)shar_mem_hdr_smcst1,  
		  shar_mem_hdr_smcst1->handle);

	smm_smcst->memManager->freeAll();

	free(smm_smcst);
}

TEST(shared_mem_create_several_times, then_shar_mem_create_should_return_null)
{
    TEST_ASSERT_NOT_NULL(smm_smcst);
    TEST_ASSERT_NOT_NULL(shar_mem_hdr_smcst1);

    TEST_ASSERT_NULL(shar_mem_hdr_smcst2);
    TEST_ASSERT_NULL(shar_mem_hdr_smcst3);
}

TEST(shared_mem_create_several_times, then_an_error_should_be_written)
{
    TEST_ASSERT_EQUAL_UINT32(errorMessages, 2);
}

TEST_GROUP_RUNNER(shared_mem_create_several_times)
{
    RUN_TEST_CASE(shared_mem_create_several_times, then_shar_mem_create_should_return_null);
    RUN_TEST_CASE(shared_mem_create_several_times, then_an_error_should_be_written);
}