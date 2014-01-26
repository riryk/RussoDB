
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "memcontainermanager.h"
#include "fakeerrorlogger.h"

TEST_GROUP(create_memory_container);

IMemContainerManager  mm_cmc;
MemoryContainer       mc_cmc;
MemoryContainer       mc_cmc1;
size_t                size_cmc;

SETUP_DEPENDENCIES(create_memory_container) 
{
    mm_cmc = (IMemContainerManager)malloc(sizeof(SIMemContainerManager));
	mm_cmc->errorLogger   = &sFakeErrorLogger;
	mm_cmc->memContCreate = memContCreate;
}

GIVEN(create_memory_container) 
{
    size_cmc = 100;
}

WHEN(create_memory_container)
{
	mc_cmc = mm_cmc->memContCreate(
		 mm_cmc,
		 NULL,
		 NULL,
		 MCT_MemorySet,
         size_cmc,
         "test",
		 malloc);

	mc_cmc1 = mm_cmc->memContCreate(
		 mm_cmc,
		 NULL,
		 mc_cmc,
		 MCT_MemorySet,
         size_cmc,
         "test",
		 malloc);
}

TEST_TEAR_DOWN(create_memory_container)
{
	free(mm_cmc);
	free(mc_cmc);
    free(mc_cmc1);
}

TEST(create_memory_container, then_new_container_must_be_created)
{
	TEST_ASSERT_NOT_NULL(mc_cmc);
	TEST_ASSERT_EQUAL_STRING(mc_cmc->name, "test"); 

    TEST_ASSERT_NOT_NULL(mc_cmc1);
    TEST_ASSERT_EQUAL_STRING(mc_cmc1->name, "test"); 

	TEST_ASSERT_EQUAL(mc_cmc1->parent, mc_cmc); 
	TEST_ASSERT_NULL(mc_cmc1->next);
	TEST_ASSERT_EQUAL(mc_cmc->childHead, mc_cmc1); 
}

TEST_GROUP_RUNNER(create_memory_container)
{
    RUN_TEST_CASE(create_memory_container, then_new_container_must_be_created);
}


