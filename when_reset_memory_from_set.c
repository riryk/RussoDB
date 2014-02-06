
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"
#include "memhelper.h"

TEST_GROUP(reset_memory_from_set);

IMemContainerManager  mm_rmfs;
MemorySet             ms_rmfs;

SETUP_DEPENDENCIES(reset_memory_from_set) 
{
    mm_rmfs = (IMemContainerManager)malloc(sizeof(SIMemContainerManager));
	mm_rmfs->errorLogger        = &sFakeErrorLogger;
	mm_rmfs->memContCreate      = memContCreate;
	mm_rmfs->memSetCreate       = memSetCreate;
	mm_rmfs->ctorMemContMan     = &ctorMemContMan;
	mm_rmfs->resetMemoryFromSet = resetMemoryFromSet;
	mm_rmfs->allocateMemory     = allocateMemory;
	mm_rmfs->dtorMemContMan     = dtorMemContMan;
}

GIVEN(reset_memory_from_set) 
{
	int          i     = 0;
    MemoryBlock  block = NULL; 
	void*        mem;
	int          intSize = sizeof(int);

	mm_rmfs->ctorMemContMan(mm_rmfs, malloc, free); 

	ms_rmfs = mm_rmfs->memSetCreate(
	     mm_rmfs,
		 topMemCont,
		 topMemCont,
		 "test",
		 0,
		 1024,
		 1024);

	unity_mem_stat_clear();

	/* We increase memory on the following value:
	 * mem1 = 10 * 150 = 1500
	 * mem2 = 10 * 800 = 8000
	 * mem3 = 20 * 200 = 4000
	 * mem = mem1 + mem2 + mem3 = 13500
	 */
	for (i = 0; i < 10; i++)
	{
	    mm_rmfs->allocateMemory(mm_rmfs, ms_rmfs, 150);
	}
    
	for (i = 0; i < 10; i++)
	{
	    mm_rmfs->allocateMemory(mm_rmfs, ms_rmfs, 800);
	}

    for (i = 0; i < 200; i++)
	{
	    mm_rmfs->allocateMemory(mm_rmfs, ms_rmfs, 20);
	}
}

WHEN(reset_memory_from_set)
{
	mm_rmfs->resetMemoryFromSet(mm_rmfs, ms_rmfs);
}

TEST_TEAR_DOWN(reset_memory_from_set)
{
	mm_rmfs->dtorMemContMan(mm_rmfs);

	free(mm_rmfs);
}

TEST(reset_memory_from_set, then_all_malloc_calls_should_be_freed)
{
	checkMallocFreeCalls();
}

TEST(reset_memory_from_set, then_more_memory_should_be_allocated)
{
    size_t  memsize = calculateTotMemSize();

	TEST_ASSERT_TRUE(memsize > 13500); 
}

TEST_GROUP_RUNNER(reset_memory_from_set)
{
    RUN_TEST_CASE(reset_memory_from_set, then_all_malloc_calls_should_be_freed);
    RUN_TEST_CASE(reset_memory_from_set, then_more_memory_should_be_allocated);
}


