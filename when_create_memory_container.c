
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"

TEST_GROUP(create_memory_container);

IMemContainerManager  mm_cmc;
MemoryContainer       mc_cmc;

SETUP_DEPENDENCIES(create_memory_container) 
{
    mm_cmc = (IMemContainerManager)malloc(sizeof(SIMemContainerManager));
	mm_cmc->errorLogger   = &sFakeErrorLogger;
	mm_cmc->memContCreate = memContCreate;
}

GIVEN(create_memory_container) 
{

}

WHEN(create_memory_container)
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


