#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"
#include <io.h>
#include "file_manager_helper.h"

TEST_GROUP(file_reopen_closed_file);

IFileManager fm_frcf;
int          fm_res; 

SETUP_DEPENDENCIES(file_reopen_closed_file) 
{
    fm_frcf = (IFileManager)malloc(sizeof(SIFileManager));
    fm_frcf->memManager        = &sFakeMemManager;
    fm_frcf->ctorFileMan       = ctorFileMan;
	fm_frcf->cacheInsert       = cacheInsert;
    fm_frcf->cacheGetFree      = cacheGetFree;
	fm_frcf->cacheRealloc      = cacheRealloc;
	fm_frcf->openFile          = openFile;
	fm_frcf->estimateFileCount = estimateFileCount;
	fm_frcf->closeRecentFile   = closeRecentFile;
    fm_frcf->cacheDelete       = cacheDelete;
	fm_frcf->openFileToCache   = openFileToCache;
	fm_frcf->reopenFile        = reopenFile;
}

GIVEN(file_reopen_closed_file) 
{
	int      i, ind;

    fm_frcf->ctorFileMan(fm_frcf);
    doCreateTestFiles(10);

	for (i = 0; i < 10; i++)
	{
	   ind = fm_frcf->openFileToCache(
	             fm_frcf, 
				 fnames[i], 
	             _O_CREAT, 
				 _O_TEXT);

	   fds[i] = (&fileCache[ind])->fileDesc;
	}

	fm_frcf->deleteFileFromCache(fm_frcf, 7);
}

WHEN(file_reopen_closed_file)
{
	fm_res = fm_frcf->reopenFile(fm_frcf, 7);
}

TEST_TEAR_DOWN(file_reopen_closed_file)
{
	fm_frcf->memManager->freeAll();
	free(fm_frcf);
    doFreeTestFiles(); 
}

TEST(file_reopen_closed_file, 
	 then_the_file_should_be_inserted_into_the_head)
{
    TEST_ASSERT_EQUAL_UINT32((&fileCache[0])->lessRecent, 7);
}

TEST_GROUP_RUNNER(file_reopen_closed_file)
{
    RUN_TEST_CASE(file_reopen_closed_file, 
		          then_the_file_should_be_inserted_into_the_head);
}





