#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"
#include <io.h>
#include "file_manager_helper.h"

TEST_GROUP(file_reopen_opened_file);

IFileManager fm_frof;
int          fm_res; 

SETUP_DEPENDENCIES(file_reopen_opened_file) 
{
    fm_frof = (IFileManager)malloc(sizeof(SIFileManager));
    fm_frof->memManager        = &sFakeMemManager;
    fm_frof->ctorFileMan       = ctorFileMan;
	fm_frof->cacheInsert       = cacheInsert;
    fm_frof->cacheGetFree      = cacheGetFree;
	fm_frof->cacheRealloc      = cacheRealloc;
	fm_frof->openFile          = openFile;
	fm_frof->estimateFileCount = estimateFileCount;
	fm_frof->closeRecentFile   = closeRecentFile;
    fm_frof->cacheDelete       = cacheDelete;
	fm_frof->openFileToCache   = openFileToCache;
	fm_frof->reopenFile        = reopenFile;
}

GIVEN(file_reopen_opened_file) 
{
	int      i, ind;

    fm_frof->ctorFileMan(fm_frof);
    doCreateTestFiles(10);

	for (i = 0; i < 10; i++)
	{
	   ind = fm_frof->openFileToCache(
	             fm_frof, 
				 fnames[i], 
	             _O_CREAT, 
				 _O_TEXT);

	   fds[i] = (&fileCache[ind])->fileDesc;
	}
}

WHEN(file_reopen_opened_file)
{
	fm_res = fm_frof->reopenFile(fm_frof, 7);
}

TEST_TEAR_DOWN(file_reopen_opened_file)
{
	fm_frof->memManager->freeAll();
	free(fm_frof);
    doFreeTestFiles(); 
}

TEST(file_reopen_opened_file, 
	 then_the_file_should_be_deleted_from_the_cache)
{
    TEST_ASSERT_EQUAL_UINT32(fm_res, FM_SUCCESS);

    TEST_ASSERT_EQUAL_UINT32((&fileCache[6])->lessRecent, 5);
    TEST_ASSERT_EQUAL_UINT32((&fileCache[6])->moreRecent, 8);
}

TEST(file_reopen_opened_file, 
	 then_the_file_should_be_inserted_into_the_cache)
{
    TEST_ASSERT_EQUAL_UINT32((&fileCache[0])->lessRecent, 7);
}

TEST_GROUP_RUNNER(file_reopen_opened_file)
{
    RUN_TEST_CASE(file_reopen_opened_file, 
		          then_the_file_should_be_deleted_from_the_cache);
    RUN_TEST_CASE(file_reopen_opened_file, 
		          then_the_file_should_be_inserted_into_the_cache);
}





