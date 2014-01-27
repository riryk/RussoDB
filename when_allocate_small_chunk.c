
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"

TEST_GROUP(allocate_small_chunk);

IMemContainerManager  mm_asc;
MemoryContainer       mc_asc;
size_t                size_asc;
void*                 mem_asc;

SETUP_DEPENDENCIES(allocate_small_chunk) 
{
    mm_asc = (IMemContainerManager)malloc(sizeof(SIMemContainerManager));
	mm_asc->errorLogger    = &sFakeErrorLogger;
	mm_asc->memContCreate  = memContCreate;
	mm_asc->allocateMemory = allocateMemory;
	mm_asc->resetMemoryFromSet = resetMemoryFromSet;
}

GIVEN(allocate_small_chunk) 
{
	MemorySet set;
    size_asc = 100;

    mc_asc = mm_asc->memContCreate(
		 mm_asc,
		 NULL,
		 NULL,
		 MCT_MemorySet,
         size_asc,
         "test",
		 malloc);

    set = (MemorySet)mc_asc;

	set->chunkMaxSize  = 1000;
	set->nextBlockSize = 32;
	set->maxBlockSize  = 32;
}

WHEN(allocate_small_chunk)
{
	mem_asc = mm_asc->allocateMemory(
		 mm_asc,
		 mc_asc,
         size_asc);
}

TEST_TEAR_DOWN(allocate_small_chunk)
{
	MemorySet set = (MemorySet)mc_asc;
	mm_asc->resetMemoryFromSet(set);
	free(mc_asc);
	free(mm_asc);
}

TEST(allocate_small_chunk, then_new_block_must_be_allocated)
{   
    MemorySet   set             = (MemorySet)mc_asc;
	MemoryBlock block           = set->blockList;
    MemoryChunk chunk           = (MemoryChunk)((char*)mem_asc - MEM_CHUNK_SIZE);

	int         freeMemSize     = (int)block->freeEnd - (int)block->freeStart;
	int         expectedMemSize = 256 
		                          - MEM_BLOCK_SIZE
								  - chunk->size
								  - MEM_CHUNK_SIZE; 

	TEST_ASSERT_NULL(block->next);

	TEST_ASSERT_EQUAL_UINT32(block->memset, set);

	TEST_ASSERT_NOT_NULL(mem_asc);
}

TEST_GROUP_RUNNER(allocate_small_chunk)
{
    RUN_TEST_CASE(allocate_small_chunk, then_new_block_must_be_allocated);
}


