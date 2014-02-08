
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"

TEST_GROUP(create_memory_set_with_malloc_error_and_null_top_context);

IMemContainerManager  mm_cmswmeantc;
MemorySet             ms_cmswmeantc;
size_t                mincontsize_cmswmeantc;

SETUP_DEPENDENCIES(create_memory_set_with_malloc_error_and_null_top_context) 
{
    mm_cmswmeantc = (IMemContainerManager)malloc(sizeof(SIMemContainerManager));
	mm_cmswmeantc->errorLogger   = &sFakeErrorLogger;
	mm_cmswmeantc->memContCreate = memContCreate;
	mm_cmswmeantc->memSetCreate  = memSetCreate;
}

GIVEN(create_memory_set_with_malloc_error_and_null_top_context) 
{
	mincontsize_cmswmeantc = 100;
	UnityMalloc_MakeMallocFailAfterCount(1);
}

WHEN(create_memory_set_with_malloc_error_and_null_top_context)
{
	ms_cmswmeantc = mm_cmswmeantc->memSetCreate(
	     mm_cmswmeantc,
		 NULL,
		 NULL,
		 "test",
		 mincontsize_cmswmeantc,
		 25,
		 20);
}

TEST_TEAR_DOWN(create_memory_set_with_malloc_error_and_null_top_context)
{
	free(mm_cmswmeantc);
	free(ms_cmswmeantc);
}

TEST(create_memory_set_with_malloc_error_and_null_top_context, 
	 then_null_top_context_should_be_asserted)
{
    TEST_ASSERT_EQUAL_UINT32(assertArgFails, 1);
}

TEST_GROUP_RUNNER(create_memory_set_with_malloc_error_and_null_top_context)
{
    RUN_TEST_CASE(create_memory_set_with_malloc_error_and_null_top_context, 
		          then_null_top_context_should_be_asserted);
}


