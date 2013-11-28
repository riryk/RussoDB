#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"
#include "file_manager_helper.h"

TEST_GROUP(file_estimate_file_count);

IFileManager fm_fefc;

int          maxToOpen_fefc1;
int          opened_fefc1;

int          maxToOpen_fefc2;
int          opened_fefc2;

SETUP_DEPENDENCIES(file_estimate_file_count) 
{
    fm_fefc = (IFileManager)malloc(sizeof(SIFileManager));
    fm_fefc->memManager        = &sFakeMemManager;
    fm_fefc->ctorFileMan       = ctorFileMan;
	fm_fefc->openFile          = openFile;
	fm_fefc->estimateFileCount = estimateFileCount;
}

GIVEN(file_estimate_file_count) 
{
    fm_fefc->ctorFileMan(fm_fefc);
	doCreateTestFiles(100);
}

WHEN(file_estimate_file_count)
{   
	int  i;
	fm_fefc->estimateFileCount(
		            fm_fefc, 
					10000, 
					&maxToOpen_fefc1, 
					&opened_fefc1);
    
    for (i = 0; i < 100; i++)
	{
	   fds[i] = fm_fefc->openFile(fm_fefc, fnames[i], _O_CREAT, _O_TEXT);  
	}

    fm_fefc->estimateFileCount(
		            fm_fefc, 
					10000, 
					&maxToOpen_fefc2, 
					&opened_fefc2);
}

TEST_TEAR_DOWN(file_estimate_file_count)
{
	fm_fefc->memManager->freeAll();
	free(fm_fefc);
	doFreeTestFiles();
}

TEST(file_estimate_file_count, then_test)
{
    TEST_ASSERT_EQUAL_UINT32(maxToOpen_fefc1, maxToOpen_fefc2 + 100);        
    TEST_ASSERT_EQUAL_UINT32(opened_fefc1, opened_fefc2 + 100);        
}

TEST_GROUP_RUNNER(file_estimate_file_count)
{
    RUN_TEST_CASE(file_estimate_file_count, then_test);
}


