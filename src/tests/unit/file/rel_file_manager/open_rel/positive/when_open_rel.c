#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"
#include "relfilemanager.h"
#include <io.h>
#include "file_manager_helper.h"
#include "snprintf.h"

TEST_GROUP(open_rel);

IRelFileManager rfm_or;
SRelData        rfm_rel;
FileSeg         rfm_seg; 
char*           rfm_start = "test_result";

SETUP_DEPENDENCIES(open_rel) 
{
    rfm_or = (IRelFileManager)malloc(sizeof(SIRelFileManager));
    rfm_or->memManager         = &sFakeMemManager;
    rfm_or->fileManager        = &sFileManager;
	rfm_or->ctorRelFileMan     = ctorRelFileMan;
	rfm_or->openRel            = openRel;
	rfm_or->createRelPart      = createRelPart;
	rfm_or->getFilePath        = getFilePath;
}

GIVEN(open_rel) 
{
	rfm_rel.relKey.node.relId      = 1;
	rfm_rel.relKey.node.databaseId = 1;
	rfm_rel.relKey.node.tblSpaceId = GLOBAL_TBL_SPACE;

	rfm_or->ctorRelFileMan(rfm_or);
}

WHEN(open_rel)
{
    rfm_seg = rfm_or->openRel(rfm_or, rfm_start, &rfm_rel, 0);
}

TEST_TEAR_DOWN(open_rel)
{
	rfm_or->closeSegm(rfm_or, rfm_seg);
	remove("test_result/global/1");

	rfm_or->memManager->freeAll();
	free(rfm_or);
}

TEST(open_rel, then_file_should_be_opened)
{
    FileSeg  seg = rfm_rel.parts[0];
	
	TEST_ASSERT_NOT_NULL(seg);
	TEST_ASSERT_TRUE(seg->find > 0);
}

TEST_GROUP_RUNNER(open_rel)
{
    RUN_TEST_CASE(open_rel, then_file_should_be_opened);
}





