
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
    size_amsc2 = 205;

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
	MemoryChunk current;
    
	int         blockHdrSize   = MEM_BLOCK_SIZE;
	int         chunkHdrSize   = MEM_CHUNK_SIZE;
    int         blockFreeSpace;

	/* First of all we round up 205
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

	/* space = 224 here. And we are going to carve it up. 
	 * The closest power of two is 128. 2^7. 
	 * [0, 2^3] is mapped to 0
	 * [2^3, 2^4] is mapped to 1
	 * [2^4, 2^5] is mapped to 2
	 * It should go to freeList[6]
	 * Then the space which is left is 224 - 128 - 16 = 80
	 * Take a power of two from it: 64 = 2^6. 
	 * It should be in freeList[5].
	 * The size which is left is 96 - 64 - 16 is less than 24.
	 */
	current = set->freelist[4];

	TEST_ASSERT_NOT_NULL(current);
    TEST_ASSERT_EQUAL(current->size, 128);
    TEST_ASSERT_EQUAL(current->sizeRequested, 0);

    current = set->freelist[3];

    TEST_ASSERT_NOT_NULL(current);
    TEST_ASSERT_EQUAL(current->size, 64);
    TEST_ASSERT_EQUAL(current->sizeRequested, 0);
}

TEST_GROUP_RUNNER(allocate_many_small_chunks_with_carving_up)
{
    RUN_TEST_CASE(allocate_many_small_chunks_with_carving_up, 
		          then_the_first_block_space_must_be_carved_up);
}


