
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"

TEST_GROUP(allocate_many_small_chunks_with_carving_up);

IMemContainerManager  mm_amsc;
MemoryContainer       mc_amsc;

size_t                size_amsc1;
size_t                size_amsc2;

void*                 mem_amsc1;
void*                 mem_amsc2;

SETUP_DEPENDENCIES(allocate_many_small_chunks_with_carving_up) 
{
    mm_amsc = (IMemContainerManager)malloc(sizeof(SIMemContainerManager));
	mm_amsc->errorLogger        = &sFakeErrorLogger;
	mm_amsc->memContCreate      = memContCreate;
	mm_amsc->allocateMemory     = allocateMemory;
	mm_amsc->resetMemoryFromSet = resetMemoryFromSet;
}

GIVEN(allocate_many_small_chunks_with_carving_up) 
{
	MemorySet set;
    size_amsc1 = 200;
    size_amsc2 = 30;

    mc_amsc = mm_amsc->memContCreate(
		 mm_amsc,
		 NULL,
		 NULL,
		 MCT_MemorySet,
         sizeof(SMemorySet),
         "test",
		 malloc);

    set = (MemorySet)mc_amsc;

	set->chunkMaxSize  = 1000;
	set->nextBlockSize = 32;
	set->maxBlockSize  = 32;
}

WHEN(allocate_many_small_chunks_with_carving_up)
{
	mem_amsc1 = mm_amsc->allocateMemory(
		 mm_amsc,
		 mc_amsc,
         size_amsc1);

    mem_amsc2 = mm_amsc->allocateMemory(
		 mm_amsc,
		 mc_amsc,
         size_amsc2);
}

TEST_TEAR_DOWN(allocate_many_small_chunks_with_carving_up)
{
	MemorySet set = (MemorySet)mc_amsc;
	mm_amsc->resetMemoryFromSet(set);

	free(mc_amsc);
	free(mm_amsc);
}

TEST(allocate_many_small_chunks_with_carving_up, 
	 then_the_first_block_space_must_be_carved_up)
{   
    MemorySet   set             = (MemorySet)mc_amsc;
	MemoryBlock block           = set->blockList;
    MemoryChunk chunk           = (MemoryChunk)((char*)mem_amsc1 - MEM_CHUNK_SIZE);
    
	int         blockHdrSize   = MEM_BLOCK_SIZE;
	int         chunkHdrSize   = MEM_CHUNK_SIZE;

	/* First of all we round up 200 
	 * to the next power of 2: 256.
	 * Then we allocate a new block with size:
	 * blockSize = 256 + MEM_BLOCK_SIZE + MEM_CHUNK_SIZE;
     * Then we round it up to the next power of two as well.
	 * And we get blockSize = 512.
	 */
    int         space = 512;
    
	/* Reduce space by Block Header size. */
	space -= blockHdrSize;  

	/* Reduce space by Chunk Header size. */
    space -= chunkHdrSize;
    
	/* Reduce space by rounded chunk space. */
    space -= 256;
}

TEST_GROUP_RUNNER(allocate_many_small_chunks_with_carving_up)
{
    RUN_TEST_CASE(allocate_many_small_chunks_with_carving_up, 
		          then_the_first_block_space_must_be_carved_up);
}


