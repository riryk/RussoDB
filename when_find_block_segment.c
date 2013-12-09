#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"
#include "relfilemanager.h"
#include <io.h>
#include "file_manager_helper.h"
#include "snprintf.h"

TEST_GROUP(find_block_segment);

IRelFileManager fbs_or;
SRelData        fbs_rel;
char*           fbs_start = "test_result";

SETUP_DEPENDENCIES(find_block_segment) 
{
    fbs_or = (IRelFileManager)malloc(sizeof(SIRelFileManager));
    fbs_or->memManager         = &sFakeMemManager;
    fbs_or->fileManager        = &sFileManager;
	fbs_or->ctorRelFileMan     = ctorRelFileMan;
	fbs_or->openRel            = openRel;
	fbs_or->createRelPart      = createRelPart;
	fbs_or->getFilePath        = getFilePath;
	fbs_or->findBlockSegm      = findBlockSegm;
}

GIVEN(find_block_segment) 
{
	fbs_rel.relKey.node.relId      = 1;
	fbs_rel.relKey.node.databaseId = 1;
	fbs_rel.relKey.node.tblSpaceId = GLOBAL_TBL_SPACE;

	fbs_or->ctorRelFileMan(fbs_or);
}

WHEN(find_block_segment)
{
	fbs_or->findBlockSegm(fbs_or, fbs_start, &fbs_rel, 0, 100, EXTENSION_CREATE, 10);
}

TEST_TEAR_DOWN(find_block_segment)
{
	fbs_or->memManager->freeAll();
	free(fbs_or);
}

TEST(find_block_segment, then_test)
{
    
}

TEST_GROUP_RUNNER(find_block_segment)
{
    RUN_TEST_CASE(find_block_segment, then_test);
}





