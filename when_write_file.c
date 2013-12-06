
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"
#include <io.h>

TEST_GROUP(write_file);

IFileManager fm_wf;
int          ind_wf;
char*        name_wf;
char*        buffer_wf;

SETUP_DEPENDENCIES(write_file) 
{
    fm_wf = (IFileManager)malloc(sizeof(SIFileManager));
    fm_wf->memManager      = &sFakeMemManager;
    fm_wf->ctorFileMan     = ctorFileMan;
	fm_wf->writeFile       = writeFile;
	fm_wf->openFileToCache = openFileToCache;
	fm_wf->cacheInsert       = cacheInsert;
    fm_wf->cacheGetFree      = cacheGetFree;
	fm_wf->cacheRealloc      = cacheRealloc;
	fm_wf->openFile          = openFile;
	fm_wf->estimateFileCount = estimateFileCount;
	fm_wf->closeRecentFile   = closeRecentFile;
    fm_wf->cacheDelete       = cacheDelete;
}

GIVEN(write_file) 
{
	name_wf = "test_result/test.txt";
	fm_wf->ctorFileMan(fm_wf);
    ind_wf = fm_wf->openFileToCache(fm_wf, name_wf, _O_CREAT, _O_TEXT);
}

WHEN(write_file)
{
    buffer_wf = "aaaa aaaa aaaa aaaa";
	fm_wf->writeFile(fm_wf, ind_wf, buffer_wf, strlen(buffer_wf));
}

TEST_TEAR_DOWN(write_file)
{
	fm_wf->memManager->freeAll();
	free(fm_wf);

	if (FILE_EXISTS(name_wf))
	{
		close(ind_wf);
		remove(name_wf);
	}
}

TEST(write_file, then_test)
{

}

TEST_GROUP_RUNNER(write_file)
{
    RUN_TEST_CASE(write_file, then_test);
}
