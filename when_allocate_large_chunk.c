
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"

TEST_GROUP(allocate_large_chunk);

IMemContainerManager  mm_alc;
MemoryContainer       mc_alc;
size_t                size_alc;
void*                 mem_alc;

SETUP_DEPENDENCIES(allocate_large_chunk) 
{
    mm_alc = (IMemContainerManager)malloc(sizeof(SIMemContainerManager));
	mm_alc->errorLogger    = &sFakeErrorLogger;
	mm_alc->memContCreate  = memContCreate;
	mm_alc->allocateMemory = allocateMemory;
	mm_alc->resetMemoryFromSet = resetMemoryFromSet;
}

GIVEN(allocate_large_chunk) 
{
    size_alc = 100;

    mc_alc = mm_alc->memContCreate(
		 mm_alc,
		 NULL,
		 NULL,
		 MCT_MemorySet,
         size_alc,
         "test",
		 malloc);
}

WHEN(allocate_large_chunk)
{
	mem_alc = mm_alc->allocateMemory(
		 mm_alc,
		 mc_alc,
         size_alc);
}

TEST_TEAR_DOWN(allocate_large_chunk)
{
	MemorySet set = (MemorySet)mc_alc;
	mm_alc->resetMemoryFromSet(set);
	free(mc_alc);
	free(mm_alc);
}

TEST(allocate_large_chunk, then_chunk_block_must_be_created)
{    
    int chunk_size_requested = size_alc;
    int chunk_size           = ALIGN_DEFAULT(chunk_size_requested);

	void*           chunkMem = mem_alc;
	MemoryChunk     chunk    = (MemoryChunk)((char*)chunkMem - MEM_CHUNK_SIZE); 

	TEST_ASSERT_NOT_NULL(chunkMem);
    TEST_ASSERT_NOT_NULL(chunk);

	TEST_ASSERT_EQUAL(chunk->memsetorchunk, mc_alc);
	TEST_ASSERT_EQUAL_INT32(chunk->size, chunk_size);
	TEST_ASSERT_EQUAL_INT32(chunk->sizeRequested, chunk_size_requested);
}

TEST(allocate_large_chunk, then_new_block_must_be_added_to_block_list)
{
	MemorySet	  set               = (MemorySet)mc_alc;
	MemoryBlock   block             = set->blockList;

    int           expectedFreeStart = (char*)block 
		                             + size_alc 
									 + MEM_BLOCK_SIZE 
									 + MEM_CHUNK_SIZE;

    TEST_ASSERT_NOT_NULL(set);

    block = set->blockList;

	TEST_ASSERT_NOT_NULL(block);
    TEST_ASSERT_NULL(block->next);
    
	TEST_ASSERT_EQUAL(block->memset, mc_alc);
	TEST_ASSERT_EQUAL(block->freeStart, block->freeEnd);

	TEST_ASSERT_EQUAL(block->freeStart, expectedFreeStart);
}

TEST_GROUP_RUNNER(allocate_large_chunk)
{
    RUN_TEST_CASE(allocate_large_chunk, then_chunk_block_must_be_created);
    RUN_TEST_CASE(allocate_large_chunk, then_new_block_must_be_added_to_block_list);
}


