
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"

TEST_GROUP(allocate_many_large_chunks);

IMemContainerManager  mm_amlc;
MemoryContainer       mc_amlc;
size_t                size_amlc;

SETUP_DEPENDENCIES(allocate_many_large_chunks) 
{
    mm_amlc = (IMemContainerManager)malloc(sizeof(SIMemContainerManager));
	mm_amlc->errorLogger    = &sFakeErrorLogger;
	mm_amlc->memContCreate  = memContCreate;
	mm_amlc->allocateMemory = allocateMemory;
	mm_amlc->resetMemoryFromSet = resetMemoryFromSet;
}

GIVEN(allocate_large_chunk) 
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

TEST_GROUP_RUNNER(allocate_large_chunk)
{
    RUN_TEST_CASE(allocate_large_chunk, then_chunk_block_must_be_created);
}


