#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"

TEST_GROUP(file_cache_delete);

IFileManager fm_fcd;
int          ind_fcd;

SETUP_DEPENDENCIES(file_cache_delete) 
{
    fm_fcd = (IFileManager)malloc(sizeof(SIFileManager));
    fm_fcd->memManager = &sFakeMemManager;
    fm_fcd->ctorFileMan = ctorFileMan;
	fm_fcd->cacheGetFree = cacheGetFree;
    fm_fcd->cacheInsert = cacheInsert;
	fm_fcd->cacheDelete = cacheDelete;
	fm_fcd->cacheRealloc = cacheRealloc;
}

GIVEN(file_cache_delete) 
{
    fm_fcd->ctorFileMan(fm_fcd);
}

WHEN(file_cache_delete)
{
	int i;
    for (i = 0; i < 10; i++)
	{
	    ind_fcd = fm_fcd->cacheGetFree(fm_fcd);
	    fm_fcd->cacheInsert(ind_fcd);
	}
	fm_fcd->cacheDelete(5);
}

TEST_TEAR_DOWN(file_cache_delete)
{
	fm_fcd->memManager->freeAll();
	free(fm_fcd);
}

/* Before deleting we have:
 * 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7 -> 8 -> 9 -> 10
 *   <-   <-   <-   <-   <-   <-   <-   <-   <-
 * After deleting item 5 we have: 
 * 1 -> 2 -> 3 -> 4 ------> 6 -> 7 -> 8 -> 9 -> 10
 *   <-   <-   <-   <------   <-   <-   <-   <-
 */
TEST(file_cache_delete, 
	 then_the_item_should_be_removed_from_the_ring)
{
	int      i;
    FCacheEl theHead           = &fileCache[0]; 
    FCacheEl theCurrent        = theHead;
    FCacheEl theNext;

    for (i = 0; i < 9; i++)
	{
		int expMore = (i >= 4) ? i + 2 : i + 1;
		int expLess = (i > 4) ? i + 1 : i; 

        TEST_ASSERT_EQUAL_UINT32(theCurrent->moreRecent, expMore);
        theNext = &fileCache[expMore];

        TEST_ASSERT_EQUAL_UINT32(theNext->lessRecent, expLess);
		theCurrent = theNext;
	}
}

TEST_GROUP_RUNNER(file_cache_delete)
{
    RUN_TEST_CASE(file_cache_delete, 
		          then_the_item_should_be_removed_from_the_ring);
}


