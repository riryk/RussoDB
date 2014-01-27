
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"

TEST_GROUP(allocate_many_large_chunks);

IMemContainerManager  mm_amlc;
MemoryContainer       mc_amlc;
size_t                size_amlc;
void*                 mem_amlc1;
void*                 mem_amlc2;
void*                 mem_amlc3;

SETUP_DEPENDENCIES(allocate_many_large_chunks) 
{
    mm_amlc = (IMemContainerManager)malloc(sizeof(SIMemContainerManager));
	mm_amlc->errorLogger    = &sFakeErrorLogger;
	mm_amlc->memContCreate  = memContCreate;
	mm_amlc->allocateMemory = allocateMemory;
	mm_amlc->resetMemoryFromSet = resetMemoryFromSet;
}

GIVEN(allocate_many_large_chunks) 
{
    size_amlc = 100;

    mc_amlc = mm_amlc->memContCreate(
		 mm_amlc,
		 NULL,
		 NULL,
		 MCT_MemorySet,
         size_amlc,
         "test",
		 malloc);
}

WHEN(allocate_many_large_chunks)
{
	mem_amlc1 = mm_amlc->allocateMemory(
		 mm_amlc,
		 mc_amlc,
         size_amlc);

    mem_amlc2 = mm_amlc->allocateMemory(
		 mm_amlc,
		 mc_amlc,
         size_amlc);

	mem_amlc3 = mm_amlc->allocateMemory(
		 mm_amlc,
		 mc_amlc,
         size_amlc);
}

TEST_TEAR_DOWN(allocate_many_large_chunks)
{
	MemorySet set = (MemorySet)mc_amlc;
	mm_amlc->resetMemoryFromSet(set);
	free(mc_amlc);
	free(mm_amlc);
}

TEST(allocate_many_large_chunks, then_chunk_blocks_must_be_created)
{   
    MemorySet   set      = (MemorySet)mc_amlc;
	MemoryBlock block    = set->blockList;
    int         blockNum = 0;

	while (block != NULL)
	{
		block = block->next;
        blockNum++;  
	}

	TEST_ASSERT_NOT_NULL(mem_amlc1);
    TEST_ASSERT_NOT_NULL(mem_amlc2);
    TEST_ASSERT_NOT_NULL(mem_amlc3);
	TEST_ASSERT_EQUAL_UINT32(blockNum, 3);
}

TEST_GROUP_RUNNER(allocate_many_large_chunks)
{
    RUN_TEST_CASE(allocate_many_large_chunks, then_chunk_blocks_must_be_created);
}


