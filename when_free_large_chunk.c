
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"
#include "memhelper.h"

TEST_GROUP(free_large_chunk);

IMemContainerManager  mm_flc;
MemorySet             ms_flc;
void*                 mem_flc;

SETUP_DEPENDENCIES(free_large_chunk) 
{
    mm_flc = (IMemContainerManager)malloc(sizeof(SIMemContainerManager));
	mm_flc->errorLogger        = &sFakeErrorLogger;
	mm_flc->memContCreate      = memContCreate;
	mm_flc->memSetCreate       = memSetCreate;
	mm_flc->ctorMemContMan     = &ctorMemContMan;
	mm_flc->resetMemoryFromSet = resetMemoryFromSet;
	mm_flc->allocateMemory     = allocateMemory;
	mm_flc->dtorMemContMan     = dtorMemContMan;
	mm_flc->freeChunk          = freeChunk;
}

GIVEN(free_large_chunk) 
{
	mm_flc->ctorMemContMan(mm_flc, malloc, free); 

	ms_flc = mm_flc->memSetCreate(
	     mm_flc,
		 topMemCont,
		 topMemCont,
		 "test",
		 0,
 		 1024,
		 1024);

	mem_flc = mm_flc->allocateMemory(mm_flc, ms_flc, 150);
    mem_flc = mm_flc->allocateMemory(mm_flc, ms_flc, 150);
    mem_flc = mm_flc->allocateMemory(mm_flc, ms_flc, 150);
}

WHEN(free_large_chunk)
{
	mm_flc->freeChunk(mm_flc, mem_flc);
}

TEST_TEAR_DOWN(free_large_chunk)
{
	mm_flc->dtorMemContMan(mm_flc);

	free(mm_flc);
}

TEST(free_large_chunk, then_the_chunk_block_must_be_freed)
{
	int          blockSize      = MEM_BLOCK_SIZE;
	int          chunkSize      = MEM_CHUNK_SIZE;
	MemoryBlock  chunkBlock     = (MemoryBlock)((char*)mem_flc - blockSize - chunkSize);

    TEST_ASSERT_EQUAL_UINT32(free_count, 1);
    TEST_ASSERT_EQUAL_UINT32(free_mem_addrs[0], (int)chunkBlock);
}

TEST(free_large_chunk, then_the_chunk_block_must_be_deleted_from_list)
{
	int          blockCount     = 0;
	MemoryBlock  block          = ms_flc->blockList;
    MemoryBlock  chunkBlock     = (char*)mem_flc - MEM_BLOCK_SIZE - MEM_CHUNK_SIZE;
    Bool         isBlockPresent = False;

	while (block != NULL)
	{
		blockCount++;
		block = block->next;
	}

    TEST_ASSERT_EQUAL_UINT32(blockCount, 2);

	block = ms_flc->blockList;
	while (block != NULL)
	{
		if (block == chunkBlock)
            isBlockPresent = True;

		block = block->next;
	}

	TEST_ASSERT_FALSE(isBlockPresent);
}

TEST_GROUP_RUNNER(free_large_chunk)
{
    RUN_TEST_CASE(free_large_chunk, then_the_chunk_block_must_be_freed);
    RUN_TEST_CASE(free_large_chunk, then_the_chunk_block_must_be_deleted_from_list);
}


