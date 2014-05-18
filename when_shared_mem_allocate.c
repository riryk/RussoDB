#include "unity_fixture.h"
#include "fakeerrorlogger.h"
#include "sharedmemmanager.h"
#include "fakememmanager.h"
#include <windows.h>
#include "sharedmem.h"
#include "spinlockmanager.h"
#include "sharedmem_helper.h"

TEST_GROUP(shared_mem_allocate);

ISharedMemManager smm_sma;
SharMemHeader     shar_mem_hdr_sma; 
size_t            size_sma;
SharMemTest       smem_test_sma;

SETUP_DEPENDENCIES(shared_mem_allocate) 
{
    smm_sma = (ISharedMemManager)malloc(sizeof(SISharedMemManager));
	smm_sma->errorLogger          = &sFakeErrorLogger;
	smm_sma->memManager           = &sFakeMemManager;
    smm_sma->sharMemCreate        = sharMemCreate;
    smm_sma->initSharMemAccess    = initSharMemAccess;
    smm_sma->allocSharedMem       = allocSharedMem;
	smm_sma->openSharedMemSegment = openSharedMemSegment;
    smm_sma->sizeMultiply         = sizeMultiply;
    smm_sma->addSize              = addSize;
	smm_sma->deleteSharedMemory   = deleteSharedMemory;
    smm_sma->sharMemCtor          = sharMemCtor;
}

GIVEN(shared_mem_allocate) 
{
    size_sma         = 1024 * 16;

	shar_mem_hdr_sma = smm_sma->sharMemCreate(smm_sma, size_sma); 
	smm_sma->sharMemCtor(smm_sma);
}

WHEN(shared_mem_allocate)
{
	smem_test_sma = 
		(SharMemTest)smm_sma->allocSharedMem(
		    smm_sma, 
			sizeof(SSharMemTest));
}

TEST_TEAR_DOWN(shared_mem_allocate)
{
	smm_sma->deleteSharedMemory(
	      smm_sma, 
		  (void*)shar_mem_hdr_sma,  
		  shar_mem_hdr_sma->handle);

	smm_sma->memManager->freeAll();

	free(smm_sma);
}

TEST(shared_mem_allocate, then_we_should_be_able_to_change_memory)
{
    TEST_ASSERT_NOT_NULL(smem_test_sma);

    smem_test_sma->field1 = 345;
    smem_test_sma->field2 = 456;
    smem_test_sma->field3 = 789;

    TEST_ASSERT_EQUAL_UINT32(smem_test_sma->field1, 345);
	TEST_ASSERT_EQUAL_UINT32(smem_test_sma->field2, 456);
	TEST_ASSERT_EQUAL_UINT32(smem_test_sma->field3, 789);
}

TEST_GROUP_RUNNER(shared_mem_allocate)
{
    RUN_TEST_CASE(shared_mem_allocate, then_we_should_be_able_to_change_memory);
}


