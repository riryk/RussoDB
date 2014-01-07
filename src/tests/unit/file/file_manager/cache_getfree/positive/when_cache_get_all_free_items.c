#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"

TEST_GROUP(file_cache_get_all_free_items);

IFileManager fm_fcgafi;
int          i;
int          new_pos_afi;

SETUP_DEPENDENCIES(file_cache_get_all_free_items) 
{
    fm_fcgafi = (IFileManager)malloc(sizeof(SIFileManager));
    fm_fcgafi->memManager = &sFakeMemManager;
    fm_fcgafi->ctorFileMan = ctorFileMan;
	fm_fcgafi->cacheRealloc = cacheRealloc;
	fm_fcgafi->cacheGetFree = cacheGetFree;
}

GIVEN(file_cache_get_all_free_items) 
{
    fm_fcgafi->ctorFileMan(fm_fcgafi);
}

WHEN(file_cache_get_all_free_items)
{  
	for (i = 0; i < 31; i++)
		new_pos_afi = fm_fcgafi->cacheGetFree(fm_fcgafi);
    new_pos_afi = fm_fcgafi->cacheGetFree(fm_fcgafi);
}

TEST_TEAR_DOWN(file_cache_get_all_free_items)
{
	fm_fcgafi->memManager->freeAll();
	free(fm_fcgafi);
}

TEST(file_cache_get_all_free_items, 
	 then_the_file_cache_must_be_reallocated)
{
	TEST_ASSERT_EQUAL_UINT32(fileCacheCount, 64);

	/* We have deleted the first item from the free list. */
	TEST_ASSERT_EQUAL_UINT32(fileCache[0].nextFree, 33); 

    for (i = 34; i < 63; i++)
       TEST_ASSERT_EQUAL_UINT32(fileCache[i].nextFree, i + 1);

    TEST_ASSERT_EQUAL_UINT32(fileCache[63].nextFree, 0);
}

TEST_GROUP_RUNNER(file_cache_get_all_free_items)
{
    RUN_TEST_CASE(file_cache_get_all_free_items, 
		          then_the_file_cache_must_be_reallocated);
}


