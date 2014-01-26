
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"

TEST_GROUP(create_cont_with_too_large_size);

IMemContainerManager  mm_ccwtls;
MemoryContainer       mc_ccwtls;

SETUP_DEPENDENCIES(create_cont_with_too_large_size) 
{
    mm_ccwtls = (IMemContainerManager)malloc(sizeof(SIMemContainerManager));
	mm_ccwtls->errorLogger   = &sFakeErrorLogger;
	mm_ccwtls->memContCreate = memContCreate;
}

GIVEN(create_cont_with_too_large_size) 
{
	UnityMalloc_MakeMallocFailAfterCount(0);
}

WHEN(create_cont_with_too_large_size)
{
	size_t  largesize = 1 << 31 - 1;

	mc_ccwtls = mm_ccwtls->memContCreate(
		 mm_ccwtls,
		 NULL,
		 NULL,
		 MCT_MemorySet,
         largesize,
         "test",
		 malloc);
}

TEST_TEAR_DOWN(create_cont_with_too_large_size)
{
	free(mm_ccwtls);
}

TEST(create_cont_with_too_large_size, then_assertion_must_fail)
{
	TEST_ASSERT_EQUAL_UINT32(assertFails, 1); 
}

TEST_GROUP_RUNNER(create_cont_with_too_large_size)
{
    RUN_TEST_CASE(create_cont_with_too_large_size, then_assertion_must_fail);
}


