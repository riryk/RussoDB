#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"
#include "relfilemanager.h"
#include <io.h>
#include "file_manager_helper.h"
#include "snprintf.h"

TEST_GROUP(open_rel_segm);

IRelFileManager rfsm_or;
SRelData        rfsm_rel;
FileSeg         rfsm_seg; 
char*           rfsm_start = "test_result";

SETUP_DEPENDENCIES(open_rel_segm) 
{
    rfsm_or = (IRelFileManager)malloc(sizeof(SIRelFileManager));
    rfsm_or->memManager         = &sFakeMemManager;
    rfsm_or->fileManager        = &sFileManager;
	rfsm_or->ctorRelFileMan     = ctorRelFileMan;
	rfsm_or->openRel            = openRel;
	rfsm_or->createRelPart      = createRelPart;
	rfsm_or->getFilePath        = getFilePath;
	rfsm_or->openRelSegm        = openRelSegm;
}

GIVEN(open_rel_segm) 
{
	rfsm_rel.relKey.node.relId      = 1;
	rfsm_rel.relKey.node.databaseId = 1;
	rfsm_rel.relKey.node.tblSpaceId = GLOBAL_TBL_SPACE;

	rfsm_or->ctorRelFileMan(rfsm_or);
}

WHEN(open_rel_segm)
{
    rfsm_seg = rfsm_or->openRelSegm(
		rfsm_or, 
		rfsm_start, 
		&rfsm_rel,
		0, 
		12, 
		_O_CREAT);
}

TEST_TEAR_DOWN(open_rel_segm)
{
	rfsm_or->closeSegm(rfsm_or, rfsm_seg);
	remove("test_result/global/1.12");

	rfsm_or->memManager->freeAll();
	free(rfsm_or);
}

TEST(open_rel_segm, then_test)
{	
	TEST_ASSERT_NOT_NULL(rfsm_seg);
	TEST_ASSERT_TRUE(rfsm_seg->find > 0);
}

TEST_GROUP_RUNNER(open_rel_segm)
{
    RUN_TEST_CASE(open_rel_segm, then_test);
}





