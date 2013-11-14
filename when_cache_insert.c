#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"

TEST_GROUP(file_cache_insert);

IFileManager fm_fci;
int          ind_fci;

SETUP_DEPENDENCIES(file_cache_insert) 
{
    fm_fci = (IFileManager)malloc(sizeof(SIFileManager));
    fm_fci->memManager = &sFakeMemManager;
    fm_fci->ctorFileMan = ctorFileMan;
	fm_fci->cacheGetFree = cacheGetFree;
    fm_fci->cacheInsert = cacheInsert;
}

GIVEN(file_cache_insert) 
{
    fm_fci->ctorFileMan(fm_fci);
}

WHEN(file_cache_insert)
{
	ind_fci = fm_fci->cacheGetFree(fm_fci);
	fm_fci->cacheInsert(ind_fci);
}

TEST_TEAR_DOWN(file_cache_insert)
{
	fm_fci->memManager->freeAll();
	free(fm_fci);
}

TEST(file_cache_insert, 
	 then_the_item_should_be_added_to_the_ring)
{
    FCacheEl theHead           = &fileCache[0]; 
	int      theLessRecentInd  = theHead->lessRecent;
    FCacheEl theLessRecent     = &fileCache[theLessRecentInd]; 

	TEST_ASSERT_EQUAL_UINT32(theHead->lessRecent, 1);
	TEST_ASSERT_EQUAL_UINT32(theHead->moreRecent, 1);

	TEST_ASSERT_EQUAL_UINT32(theLessRecent->lessRecent, 0);
	TEST_ASSERT_EQUAL_UINT32(theLessRecent->moreRecent, 0);
}

TEST_GROUP_RUNNER(file_cache_insert)
{
    RUN_TEST_CASE(file_cache_insert, 
		          then_the_item_should_be_added_to_the_ring);
}


