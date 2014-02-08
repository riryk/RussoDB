
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"

TEST_GROUP(create_memory_set);

IMemContainerManager  mm_cms;
MemorySet             ms_cms;
size_t                size_cms;

SETUP_DEPENDENCIES(create_memory_set) 
{
    mm_cms = (IMemContainerManager)malloc(sizeof(SIMemContainerManager));
	mm_cms->errorLogger   = &sFakeErrorLogger;
	mm_cms->memContCreate = memContCreate;
	mm_cms->memSetCreate  = memSetCreate;
}

GIVEN(create_memory_set) 
{
    size_cms = 100;
}

WHEN(create_memory_set)
{
	ms_cms = mm_cms->memSetCreate(
	     mm_cms,
		 NULL,
		 NULL,
		 "test",
		 15,
		 25,
		 20);
}

TEST_TEAR_DOWN(create_memory_set)
{
	free(mm_cms);
	free(ms_cms);
}

TEST(create_memory_set, then_init_and_max_block_size_must_be_checked)
{
	TEST_ASSERT_EQUAL_UINT32(ms_cms->initBlockSize, 1024);
	TEST_ASSERT_EQUAL_UINT32(ms_cms->maxBlockSize, 1024);
	TEST_ASSERT_EQUAL_UINT32(ms_cms->nextBlockSize, 1024);
}

TEST(create_memory_set, then_chunk_max_size_must_be_adjusted)
{
	TEST_ASSERT_EQUAL_UINT32(ms_cms->chunkMaxSize, 128);
}

TEST_GROUP_RUNNER(create_memory_set)
{
    RUN_TEST_CASE(create_memory_set, then_init_and_max_block_size_must_be_checked);
}


