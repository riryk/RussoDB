
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"

TEST_GROUP(allocate_small_large_chunk);

IMemContainerManager  mm_aslc;
MemorySet             ms_aslc;

SETUP_DEPENDENCIES(allocate_small_large_chunk) 
{
    mm_aslc = (IMemContainerManager)malloc(sizeof(SIMemContainerManager));
	mm_aslc->errorLogger        = &sFakeErrorLogger;
	mm_aslc->memContCreate      = memContCreate;
	mm_aslc->memSetCreate       = memSetCreate;
	mm_aslc->ctorMemContMan     = &ctorMemContMan;
	mm_aslc->resetMemoryFromSet = resetMemoryFromSet;
	mm_aslc->allocateMemory     = allocateMemory;
	mm_aslc->dtorMemContMan     = dtorMemContMan;
}

GIVEN(allocate_small_large_chunk) 
{
	mm_aslc->ctorMemContMan(mm_aslc, malloc, free); 

	ms_aslc = mm_aslc->memSetCreate(
	     mm_aslc,
		 topMemCont,
		 topMemCont,
		 "test",
		 0,
		 1024,
		 1024);

	unity_mem_stat_clear();

    mm_aslc->allocateMemory(mm_aslc, ms_aslc, 20);
	mm_aslc->allocateMemory(mm_aslc, ms_aslc, 150);
}

WHEN(allocate_small_large_chunk)
{
	mm_aslc->allocateMemory(mm_aslc, ms_aslc, 20);
    mm_aslc->allocateMemory(mm_aslc, ms_aslc, 20);
	mm_aslc->allocateMemory(mm_aslc, ms_aslc, 150);
}

TEST_TEAR_DOWN(allocate_small_large_chunk)
{
	mm_aslc->dtorMemContMan(mm_aslc);
	free(mm_aslc);
	unity_mem_stat_clear();
}

TEST(allocate_small_large_chunk, then_test1)
{
	//checkMallocFreeCalls();
}

TEST(allocate_small_large_chunk, then_test2)
{
    //size_t  memsize = calculateTotMemSize();

	//TEST_ASSERT_TRUE(memsize > 13500); 
}

TEST_GROUP_RUNNER(allocate_small_large_chunk)
{
    RUN_TEST_CASE(allocate_small_large_chunk, then_test1);
    RUN_TEST_CASE(allocate_small_large_chunk, then_test2);
}
