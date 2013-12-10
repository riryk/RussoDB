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
FileSeg         fbs_seg_cur;
FileSeg         fbs_seg;


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
	fbs_or->openRelSegm        = openRelSegm;
	fbs_or->closeSegm          = closeSegm;
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
	fbs_seg_cur = fbs_or->findBlockSegm(fbs_or, fbs_start, &fbs_rel, 0, 100, EXTENSION_CREATE, 10);
}

TEST_TEAR_DOWN(find_block_segment)
{
    fbs_seg     = fbs_or->openRel(fbs_or, fbs_start, &fbs_rel, FILE_PART_MAIN);
	fbs_seg_cur = fbs_seg->next;

	while (fbs_seg_cur != NULL)
	{
		close(fileCache[fbs_seg_cur->find].fileDesc);
		remove(fbs_seg_cur->fname);
		fbs_seg_cur = fbs_seg_cur->next;
	}

	fbs_or->closeSegm(fbs_or, fbs_seg);
	remove("test_result/global/1");

	fbs_or->memManager->freeAll();
	free(fbs_or);
}

TEST(find_block_segment, then_all_neded_segments_should_be_created)
{
	Bool fexists;
	int  segCount = 0;

    fbs_seg = fbs_or->openRel(fbs_or, fbs_start, &fbs_rel, FILE_PART_MAIN);

	while (fbs_seg != NULL)
	{
		fexists = FILE_EXISTS(fbs_seg->fname);
        TEST_ASSERT_TRUE(fexists);

		fbs_seg = fbs_seg->next;
        segCount++;
	}

	TEST_ASSERT_EQUAL_INT(segCount, 11);
}

TEST_GROUP_RUNNER(find_block_segment)
{
    RUN_TEST_CASE(find_block_segment, then_all_neded_segments_should_be_created);
}





