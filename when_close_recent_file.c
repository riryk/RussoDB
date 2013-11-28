#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"
#include "file_manager_helper.h"

TEST_GROUP(file_close_recent_file);

IFileManager fm_fcrf;
int          ind_fcrf;
Bool         close_res;

SETUP_DEPENDENCIES(file_close_recent_file) 
{
    fm_fcrf = (IFileManager)malloc(sizeof(SIFileManager));
    fm_fcrf->memManager      = &sFakeMemManager;
    fm_fcrf->ctorFileMan     = ctorFileMan;
	fm_fcrf->cacheInsert     = cacheInsert;
	fm_fcrf->cacheGetFree    = cacheGetFree;
	fm_fcrf->cacheRealloc    = cacheRealloc;
	fm_fcrf->openFile        = openFile;
	fm_fcrf->closeRecentFile = closeRecentFile;
	fm_fcrf->cacheDelete     = cacheDelete;
}

GIVEN(file_close_recent_file) 
{
	int       i;
	int       theMostRecentInd;
    FCacheEl  theMostRecent;
	int       fd;

    fm_fcrf->ctorFileMan(fm_fcrf); 
    doCreateTestFiles(10);

	for (i = 0; i < 10; i++)
	{
		int        ind = fm_fcrf->cacheGetFree(fm_fcrf);
	    FCacheEl   it  = &fileCache[ind];

	    it->fileDesc   = fds[i] = fm_fcrf->openFile(
			                         fm_fcrf, 
									 fnames[i], 
									 _O_CREAT | O_RDWR, 
									 _O_TEXT);
		fm_fcrf->cacheInsert(ind);
		fileCount++;
	}

	theMostRecentInd = (&fileCache[0])->moreRecent;
    theMostRecent    = &fileCache[theMostRecentInd];
    fd               = theMostRecent->fileDesc;

	write(fd, "test test test test", 19);
	write(fd, "aaaa", 4);
	lseek(fd, 19, SEEK_SET);

	_commit(fd);
}

WHEN(file_close_recent_file)
{ 
	close_res = fm_fcrf->closeRecentFile(fm_fcrf);
}

TEST_TEAR_DOWN(file_close_recent_file)
{
	int alreadyClosed[1] = { (&fileCache[1])->fileDesc };
	fm_fcrf->memManager->freeAll();

	free(fm_fcrf);
	doFreeNotClosedTestFiles(alreadyClosed, 1);
}

TEST(file_close_recent_file, then_the_most_recent_should_be_deleted_from_the_ring)
{   
    FCacheEl   theHead    = &fileCache[0];
	int        theMostInd = theHead->moreRecent;
    FCacheEl   theMost    = &fileCache[theMostInd];

    TEST_ASSERT_EQUAL_UINT32(theMostInd, 2);   
	TEST_ASSERT_EQUAL_UINT32(theMost->lessRecent, 0);   
}

TEST(file_close_recent_file, then_the_seek_pos_should_be_saved)
{
	TEST_ASSERT_EQUAL_UINT32((&fileCache[1])->seekPos, 19);
}

TEST(file_close_recent_file, then_the_file_descriptor_should_be_closed)
{
}

TEST_GROUP_RUNNER(file_close_recent_file)
{
    RUN_TEST_CASE(file_close_recent_file, then_the_most_recent_should_be_deleted_from_the_ring);
    RUN_TEST_CASE(file_close_recent_file, then_the_seek_pos_should_be_saved);
    RUN_TEST_CASE(file_close_recent_file, then_the_file_descriptor_should_be_closed);
}


