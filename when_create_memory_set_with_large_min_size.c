
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"

TEST_GROUP(create_memory_set_with_large_min_size);

IMemContainerManager  mm_cmswlms;
MemorySet             ms_cmswlms;
size_t                mincontsize_cmswlms;

SETUP_DEPENDENCIES(create_memory_set_with_large_min_size) 
{
    mm_cmswlms = (IMemContainerManager)malloc(sizeof(SIMemContainerManager));
	mm_cmswlms->errorLogger   = &sFakeErrorLogger;
	mm_cmswlms->memContCreate = memContCreate;
	mm_cmswlms->memSetCreate  = memSetCreate;
}

GIVEN(create_memory_set_with_large_min_size) 
{
	mincontsize_cmswlms = 100;
}

WHEN(create_memory_set_with_large_min_size)
{
	ms_cmswlms = mm_cmswlms->memSetCreate(
	     mm_cmswlms,
		 NULL,
		 NULL,
		 "test",
		 mincontsize_cmswlms,
		 25,
		 20);
}

TEST_TEAR_DOWN(create_memory_set_with_large_min_size)
{
	free(mm_cmswlms);
	free(ms_cmswlms);
}

TEST(create_memory_set_with_large_min_size, then_new_block_must_be_created)
{
	MemoryBlock  block           = ms_cmswlms->blockList;
	int          freeMem         = block->freeEnd - block->freeStart;
	int          expectedFreeMem = AlignDefault(mincontsize_cmswlms) - MEM_BLOCK_SIZE;

	TEST_ASSERT_NOT_NULL(block);

	TEST_ASSERT_EQUAL_UINT32(block->memset, ms_cmswlms);
    TEST_ASSERT_EQUAL_UINT32(freeMem, expectedFreeMem);

	TEST_ASSERT_NULL(block->next);  
}

TEST_GROUP_RUNNER(create_memory_set_with_large_min_size)
{
    RUN_TEST_CASE(create_memory_set_with_large_min_size, then_new_block_must_be_created);
}


