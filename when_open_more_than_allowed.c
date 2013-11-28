#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"
#include <io.h>
#include "file_manager_helper.h"

TEST_GROUP(file_open_more_than_allowed);

IFileManager fm_fomta;
int          maxToOpen_fomta;
int          opened_fomta;

SETUP_DEPENDENCIES(file_open_more_than_allowed) 
{
    fm_fomta = (IFileManager)malloc(sizeof(SIFileManager));
    fm_fomta->memManager        = &sFakeMemManager;
    fm_fomta->ctorFileMan       = ctorFileMan;
	fm_fomta->cacheInsert       = cacheInsert;
    fm_fomta->cacheGetFree      = cacheGetFree;
	fm_fomta->cacheRealloc      = cacheRealloc;
	fm_fomta->openFile          = openFile;
	fm_fomta->estimateFileCount = estimateFileCount;
	fm_fomta->closeRecentFile   = closeRecentFile;
    fm_fomta->cacheDelete       = cacheDelete;
}

GIVEN(file_open_more_than_allowed) 
{
	int i;

    fm_fomta->ctorFileMan(fm_fomta);

    fm_fomta->estimateFileCount(
		            fm_fomta, 
					10000, 
					&maxToOpen_fomta, 
					&opened_fomta);

    doCreateTestFiles(maxToOpen_fomta);

	for (i = 0; i < maxToOpen_fomta; i++)
	{
	   int        ind = fm_fomta->cacheGetFree(fm_fomta);
	   FCacheEl   it  = &fileCache[ind];
	   it->fileDesc   = fds[i] = fm_fomta->openFile(fm_fomta, fnames[i], _O_CREAT, _O_TEXT);  
	   fm_fomta->cacheInsert(ind);
	   fileCount++;
	}
}

WHEN(file_open_more_than_allowed)
{
	char*  new_file = "test_result/morethanallowed.txt";
	fm_fomta->openFile(fm_fomta, new_file, _O_CREAT, _O_TEXT);
    fnames[0] = new_file;
}

TEST_TEAR_DOWN(file_open_more_than_allowed)
{
	fm_fomta->memManager->freeAll();
	free(fm_fomta);
	doFreeTestFiles();
	remove("test_result/test0.txt");
}

TEST(file_open_more_than_allowed, 
	 then_the_most_recent_file_should_be_removed)
{
    TEST_ASSERT_EQUAL_UINT32((&fileCache[0])->moreRecent, 2);
}

TEST_GROUP_RUNNER(file_open_more_than_allowed)
{
    RUN_TEST_CASE(file_open_more_than_allowed, 
		          then_the_most_recent_file_should_be_removed);
}
