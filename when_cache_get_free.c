#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"

TEST_GROUP(file_cache_get_free);

IFileManager fm_fcgf;
int          new_pos;

SETUP_DEPENDENCIES(file_cache_get_free) 
{
    fm_fcgf = (IFileManager)malloc(sizeof(SIFileManager));
    fm_fcgf->memManager = &sFakeMemManager;
    fm_fcgf->ctorFileMan = ctorFileMan;
	fm_fcgf->cacheRealloc = cacheRealloc;
	fm_fcgf->cacheGetFree = cacheGetFree;
}

GIVEN(file_cache_get_free) 
{
    fm_fcgf->ctorFileMan(fm_fcgf);
}

WHEN(file_cache_get_free)
{
	new_pos = fm_fcgf->cacheGetFree(fm_fcgf);
}

TEST_TEAR_DOWN(file_cache_get_free)
{
	fm_fcgf->memManager->freeAll();
	free(fm_fcgf);
}

TEST(file_cache_get_free, then_the_first_free_item_must_be_removed)
{
    TEST_ASSERT_EQUAL_UINT32(new_pos, 1);   
	TEST_ASSERT_EQUAL_UINT32(fileCache[0].nextFree, 2);
}

TEST_GROUP_RUNNER(file_cache_get_free)
{
    RUN_TEST_CASE(file_cache_get_free, 
		          then_the_first_free_item_must_be_removed);
}


