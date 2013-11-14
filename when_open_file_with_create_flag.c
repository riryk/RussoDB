#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"
#include <io.h>

TEST_GROUP(file_open_with_create_flag);

IFileManager fm_fowcf;
char*        fname_fowcf = "test_result/test.txt";
int          fd_fowcf;

SETUP_DEPENDENCIES(file_open_with_create_flag) 
{
    fm_fowcf = (IFileManager)malloc(sizeof(SIFileManager));
    fm_fowcf->memManager = &sFakeMemManager;
    fm_fowcf->ctorFileMan = ctorFileMan;
	fm_fowcf->openFile = openFile;
}

GIVEN(file_open_with_create_flag) 
{
    fm_fowcf->ctorFileMan(fm_fowcf);
}

WHEN(file_open_with_create_flag)
{
	fd_fowcf = fm_fowcf->openFile(fm_fowcf, fname_fowcf, _O_CREAT, _O_TEXT);
}

TEST_TEAR_DOWN(file_open_with_create_flag)
{
	int closeRes, removeRes;

	fm_fowcf->memManager->freeAll();
	free(fm_fowcf);

	if (FILE_EXISTS(fname_fowcf))
	{
		closeRes  = close(fd_fowcf);
		removeRes = remove(fname_fowcf);
	}
}

TEST(file_open_with_create_flag, then_the_file_should_be_created_on_disk)
{
    Bool isOnDisk;

	TEST_ASSERT_NOT_EQUAL(fd_fowcf, FILE_INVALID);

	isOnDisk = FILE_EXISTS(fname_fowcf);
	TEST_ASSERT_TRUE(isOnDisk);
}

TEST_GROUP_RUNNER(file_open_with_create_flag)
{
    RUN_TEST_CASE(file_open_with_create_flag, 
		          then_the_file_should_be_created_on_disk);
}
