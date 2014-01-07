#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"

TEST_GROUP(file_cache_realloc);

IFileManager fm_fcr;
int          i;

SETUP_DEPENDENCIES(file_cache_realloc) 
{
    fm_fcr = (IFileManager)malloc(sizeof(SIFileManager));
    fm_fcr->memManager = &sFakeMemManager;
    fm_fcr->ctorFileMan = ctorFileMan;
	fm_fcr->cacheRealloc = cacheRealloc;
}

GIVEN(file_cache_realloc) 
{
    fm_fcr->ctorFileMan(fm_fcr);
}

WHEN(file_cache_realloc)
{
	fm_fcr->cacheRealloc(fm_fcr);
}

TEST_TEAR_DOWN(file_cache_realloc)
{
	fm_fcr->memManager->freeAll();
	free(fm_fcr);
}

TEST(file_cache_realloc, then_minimal_items_should_be_allocated)
{
    TEST_ASSERT_EQUAL_UINT32(fileCacheCount, 32);
	TEST_ASSERT_EQUAL_UINT32(fileCache[0].nextFree, 1);

    for (i = 1; i < 30; i++)
       TEST_ASSERT_EQUAL_UINT32(fileCache[i].nextFree, i + 1);

    TEST_ASSERT_EQUAL_UINT32(fileCache[31].nextFree, 0);
}

TEST_GROUP_RUNNER(file_cache_realloc)
{
    RUN_TEST_CASE(file_cache_realloc, then_minimal_items_should_be_allocated);
}


