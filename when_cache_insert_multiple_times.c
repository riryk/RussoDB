#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"

TEST_GROUP(file_cache_insert_multiple_times);

IFileManager fm_fcimt;
int          ind_fcimt;

SETUP_DEPENDENCIES(file_cache_insert_multiple_times) 
{
    fm_fcimt = (IFileManager)malloc(sizeof(SIFileManager));
    fm_fcimt->memManager = &sFakeMemManager;
    fm_fcimt->ctorFileMan = ctorFileMan;
	fm_fcimt->cacheGetFree = cacheGetFree;
    fm_fcimt->cacheInsert = cacheInsert;
}

GIVEN(file_cache_insert_multiple_times) 
{
    fm_fcimt->ctorFileMan(fm_fcimt);
}

WHEN(file_cache_insert_multiple_times)
{
	int i;
    for (i = 0; i < 10; i++)
	{
	    ind_fcimt = fm_fcimt->cacheGetFree(fm_fcimt);
	    fm_fcimt->cacheInsert(ind_fcimt);
	}
}

TEST_TEAR_DOWN(file_cache_insert_multiple_times)
{
	fm_fcimt->memManager->freeAll();
	free(fm_fcimt);
}

TEST(file_cache_insert_multiple_times, 
	 then_the_items_should_be_added_to_the_ring)
{
	int      i;
    FCacheEl theHead           = &fileCache[0]; 
    FCacheEl theCurrent        = theHead;
    FCacheEl theNext;

    for (i = 0; i < 10; i++)
	{
        TEST_ASSERT_EQUAL_UINT32(theCurrent->moreRecent, i + 1);
        theNext = &fileCache[i + 1];
        TEST_ASSERT_EQUAL_UINT32(theNext->lessRecent, i);
		theCurrent = theNext;
	}
}

TEST_GROUP_RUNNER(file_cache_insert_multiple_times)
{
    RUN_TEST_CASE(file_cache_insert_multiple_times, 
		          then_the_items_should_be_added_to_the_ring);
}


