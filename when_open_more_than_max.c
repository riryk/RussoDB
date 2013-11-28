#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"
#include <io.h>
#include "file_manager_helper.h"

TEST_GROUP(file_open_to_cache_more_than_max);

IFileManager fm_fotcmtm;
int          new_fd_fotcmtm;
char*        new_fname_fotcmtm;

SETUP_DEPENDENCIES(file_open_to_cache_more_than_max) 
{
    fm_fotcmtm = (IFileManager)malloc(sizeof(SIFileManager));
    fm_fotcmtm->memManager        = &sFakeMemManager;
    fm_fotcmtm->ctorFileMan       = ctorFileMan;
	fm_fotcmtm->cacheInsert       = cacheInsert;
    fm_fotcmtm->cacheGetFree      = cacheGetFree;
	fm_fotcmtm->cacheRealloc      = cacheRealloc;
	fm_fotcmtm->openFile          = openFile;
	fm_fotcmtm->estimateFileCount = estimateFileCount;
	fm_fotcmtm->closeRecentFile   = closeRecentFile;
    fm_fotcmtm->cacheDelete       = cacheDelete;
	fm_fotcmtm->openFileToCache   = openFileToCache;
}

GIVEN(file_open_to_cache_more_than_max) 
{
	int      i, ind;

    fm_fotcmtm->ctorFileMan(fm_fotcmtm);
    doCreateTestFiles(fileMaxCount);

	for (i = 0; i < fileMaxCount; i++)
	{
	   ind = fm_fotcmtm->openFileToCache(
	             fm_fotcmtm, 
				 fnames[i], 
	             _O_CREAT, 
				 _O_TEXT);

	   fds[i] = (&fileCache[ind])->fileDesc;
	}
}

WHEN(file_open_to_cache_more_than_max)
{
	int    ind;
    new_fname_fotcmtm   = "test_result/morethanallowed.txt";
	ind                 = fm_fotcmtm->openFileToCache(
	                         fm_fotcmtm, 
				             new_fname_fotcmtm, 
	                         _O_CREAT, 
				             _O_TEXT);
	new_fd_fotcmtm = (&fileCache[ind])->fileDesc;
}

TEST_TEAR_DOWN(file_open_to_cache_more_than_max)
{
	int   alreadyClosed[1] = { "test_result/test0.txt" };
	fm_fotcmtm->memManager->freeAll();
	free(fm_fotcmtm);
    doFreeTestFiles(); 
	remove(new_fname_fotcmtm);
}

TEST(file_open_to_cache_more_than_max, 
	 then_the_most_recent_file_should_be_removed)
{
    TEST_ASSERT_EQUAL_UINT32((&fileCache[0])->moreRecent, 2);
}

TEST_GROUP_RUNNER(file_open_to_cache_more_than_max)
{
    RUN_TEST_CASE(file_open_to_cache_more_than_max, 
		          then_the_most_recent_file_should_be_removed);
}
