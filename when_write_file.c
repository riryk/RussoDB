
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"
#include <io.h>

TEST_GROUP(write_file);

IFileManager fm_wf;
int          ind_wf;
int          fd_wf;
char*        name_wf;
char*        buffer_wf;
int          buffer_len;

SETUP_DEPENDENCIES(write_file) 
{
    fm_wf = (IFileManager)malloc(sizeof(SIFileManager));
    fm_wf->memManager        = &sFakeMemManager;
    fm_wf->ctorFileMan       = ctorFileMan;
	fm_wf->writeFile         = writeFile;
	fm_wf->openFileToCache   = openFileToCache;
	fm_wf->cacheInsert       = cacheInsert;
    fm_wf->cacheGetFree      = cacheGetFree;
	fm_wf->cacheRealloc      = cacheRealloc;
	fm_wf->openFile          = openFile;
	fm_wf->estimateFileCount = estimateFileCount;
	fm_wf->closeRecentFile   = closeRecentFile;
    fm_wf->cacheDelete       = cacheDelete;
	fm_wf->reopenFile        = reopenFile;
}

GIVEN(write_file) 
{
	name_wf = "test_result/test.txt";
	fm_wf->ctorFileMan(fm_wf);
    ind_wf = fm_wf->openFileToCache(fm_wf, name_wf, _O_CREAT | O_RDWR, _O_TEXT);
}

WHEN(write_file)
{
    buffer_wf  = "aaaa aaaa aaaa aaaa";
	buffer_len = strlen(buffer_wf);
	fm_wf->writeFile(fm_wf, ind_wf, buffer_wf, buffer_len);
	fd_wf = fileCache[ind_wf].fileDesc;
    _commit(fd_wf);
}

TEST_TEAR_DOWN(write_file)
{
	fm_wf->memManager->freeAll();
	free(fm_wf);

	if (FILE_EXISTS(name_wf))
	{
		close(fd_wf);
		remove(name_wf);
	}
}

TEST(write_file, then_the_block_should_be_written_to_the_file)
{
    
}

TEST_GROUP_RUNNER(write_file)
{
    RUN_TEST_CASE(write_file, then_the_block_should_be_written_to_the_file);
}
