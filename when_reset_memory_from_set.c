
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"

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
}

GIVEN(reset_memory_from_set) 
{
	int          i     = 0;
    MemoryBlock  block = NULL; 

	mm_rmfs->ctorMemContMan(mm_rmfs, malloc, free); 

	ms_rmfs = mm_rmfs->memSetCreate(
	     mm_rmfs,
		 topMemCont,
		 topMemCont,
		 "test",
		 0,
		 1024,
		 1024);

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
	mm_rmfs->resetMemoryFromSet(mm_rmfs, topMemCont);

	free(topMemCont);
	free(mm_rmfs);
}

TEST(reset_memory_from_set, then_test)
{
    TEST_ASSERT_EQUAL_UINT32(errorMessages, 1);
}

TEST_GROUP_RUNNER(reset_memory_from_set)
{
    RUN_TEST_CASE(reset_memory_from_set, then_test);
}


