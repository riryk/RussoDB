#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"

TEST_GROUP(file_cache_get_free_several_times);

IFileManager fm_fcgfst;

int          new_pos_st_1;
int          new_pos_st_2;
int          new_pos_st_3;
int          new_pos_st_4;
int          new_pos_st_5;

SETUP_DEPENDENCIES(file_cache_get_free_several_times) 
{
    fm_fcgfst = (IFileManager)malloc(sizeof(SIFileManager));
    fm_fcgfst->memManager = &sFakeMemManager;
    fm_fcgfst->ctorFileMan = ctorFileMan;
	fm_fcgfst->cacheRealloc = cacheRealloc;
	fm_fcgfst->cacheGetFree = cacheGetFree;
}

GIVEN(file_cache_get_free_several_times) 
{
    fm_fcgfst->ctorFileMan(fm_fcgfst);
}

WHEN(file_cache_get_free_several_times)
{
	new_pos_st_1 = fm_fcgfst->cacheGetFree(fm_fcgfst);
    new_pos_st_2 = fm_fcgfst->cacheGetFree(fm_fcgfst);
    new_pos_st_3 = fm_fcgfst->cacheGetFree(fm_fcgfst);
    new_pos_st_4 = fm_fcgfst->cacheGetFree(fm_fcgfst);
    new_pos_st_5 = fm_fcgfst->cacheGetFree(fm_fcgfst);
}

TEST_TEAR_DOWN(file_cache_get_free_several_times)
{
	fm_fcgfst->memManager->freeAll();
	free(fm_fcgfst);
}

TEST(file_cache_get_free_several_times, 
	 then_the_first_free_item_must_be_removed)
{
    TEST_ASSERT_EQUAL_UINT32(new_pos_st_1, 1);   
    TEST_ASSERT_EQUAL_UINT32(new_pos_st_2, 2);   
    TEST_ASSERT_EQUAL_UINT32(new_pos_st_3, 3);   
    TEST_ASSERT_EQUAL_UINT32(new_pos_st_4, 4);   
    TEST_ASSERT_EQUAL_UINT32(new_pos_st_5, 5); 

	TEST_ASSERT_EQUAL_UINT32(fileCache[0].nextFree, 6);
}

TEST_GROUP_RUNNER(file_cache_get_free_several_times)
{
    RUN_TEST_CASE(file_cache_get_free_several_times, 
		          then_the_first_free_item_must_be_removed);
}


