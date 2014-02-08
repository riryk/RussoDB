
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"

TEST_GROUP(create_memory_set_with_malloc_error);

IMemContainerManager  mm_cmswme;
MemorySet             ms_cmswme;
size_t                mincontsize_cmswme;

SETUP_DEPENDENCIES(create_memory_set_with_malloc_error) 
{
    mm_cmswme = (IMemContainerManager)malloc(sizeof(SIMemContainerManager));
	mm_cmswme->errorLogger    = &sFakeErrorLogger;
	mm_cmswme->memContCreate  = memContCreate;
	mm_cmswme->memSetCreate   = memSetCreate;
	mm_cmswme->ctorMemContMan = &ctorMemContMan;
}

GIVEN(create_memory_set_with_malloc_error) 
{
	mincontsize_cmswme = 100;

	mm_cmswme->ctorMemContMan(mm_cmswme, malloc, free); 

	UnityMalloc_MakeMallocFailAfterCount(0);
}

WHEN(create_memory_set_with_malloc_error)
{
	ms_cmswme = mm_cmswme->memSetCreate(
	     mm_cmswme,
		 topMemCont,
		 topMemCont,
		 "test",
		 mincontsize_cmswme,
		 25,
		 20);
}

TEST_TEAR_DOWN(create_memory_set_with_malloc_error)
{
	mm_cmswme->resetMemoryFromSet(mm_cmswme, topMemCont);

	free(topMemCont);
	free(mm_cmswme);
}

TEST(create_memory_set_with_malloc_error, then_log_message_must_be_written)
{
    TEST_ASSERT_EQUAL_UINT32(errorMessages, 1);
}

TEST_GROUP_RUNNER(create_memory_set_with_malloc_error)
{
    RUN_TEST_CASE(create_memory_set_with_malloc_error, 
		          then_log_message_must_be_written);
}


