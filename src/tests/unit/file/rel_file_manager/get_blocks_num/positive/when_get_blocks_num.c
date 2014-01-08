#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"
#include "relfilemanager.h"
#include <io.h>
#include "file_manager_helper.h"
#include "snprintf.h"

TEST_GROUP(get_blocks_num);

IRelFileManager gbn_or;
SRelData        gbn_rel;
char*           gbn_start = "test_result";
int             gbn_blnum;

FileSeg         gbn_part0;
FileSeg         gbn_part1;
FileSeg         gbn_part2; 
FileSeg         gbn_part3;

SETUP_DEPENDENCIES(get_blocks_num) 
{
    gbn_or = (IRelFileManager)malloc(sizeof(SIRelFileManager));
    gbn_or->memManager         = &sFakeMemManager;
    gbn_or->fileManager        = &sFileManager;
	gbn_or->ctorRelFileMan     = ctorRelFileMan;
	gbn_or->openRel            = openRel;
	gbn_or->createRelPart      = createRelPart;
	gbn_or->getFilePath        = getFilePath;
	gbn_or->openRelSegm        = openRelSegm;
	gbn_or->closeSegm          = closeSegm;
}

FileSeg gbn_open_rel_segm(int i)
{
    return gbn_or->openRelSegm(
		               gbn_or, 
		               gbn_start, 
					   &gbn_rel, 
					   0, 
					   i, 
					   _O_CREAT);
}

GIVEN(get_blocks_num) 
{
	int fd, i;

	gbn_or->ctorRelFileMan(gbn_or);

	gbn_rel.relKey.node.relId      = 1;
	gbn_rel.relKey.node.databaseId = 1;
	gbn_rel.relKey.node.tblSpaceId = GLOBAL_TBL_SPACE;

	gbn_part0 = gbn_open_rel_segm(0);
    gbn_part1 = gbn_open_rel_segm(1);
	gbn_part2 = gbn_open_rel_segm(2);
	gbn_part3 = gbn_open_rel_segm(3);
    
	fd = fileCache[gbn_part3->find].fileDesc;

	for (i = 0; i < 10 * (1 << 13); i++)
        write(fd, "R", 1);

    _commit(fd);

	gbn_or->closeSegm(gbn_or, gbn_part3);

	gbn_part0->next = gbn_part1;
    gbn_part1->next = gbn_part2;
    gbn_part2->next = gbn_part3;
	gbn_part3->next = NULL;

	gbn_rel.parts[0] = gbn_part0;
}

WHEN(get_blocks_num)
{
	gbn_blnum = getBlocksNum(
        gbn_or,
        gbn_start,
	    &gbn_rel,
	    0,
		REL_SEGM_SIZE);
}

TEST_TEAR_DOWN(get_blocks_num)
{
	gbn_or->closeSegm(gbn_or, gbn_part0);
	remove("test_result/global/1");

    gbn_or->closeSegm(gbn_or, gbn_part1);
	remove("test_result/global/1.1");

    gbn_or->closeSegm(gbn_or, gbn_part2);
	remove("test_result/global/1.2");

    gbn_or->closeSegm(gbn_or, gbn_part3);
	remove("test_result/global/1.3");

	gbn_or->memManager->freeAll();
	free(gbn_or);
}

TEST(get_blocks_num, then_blocks_number_should_be_calculated)
{	
	int expected = gbn_blnum - 3 * REL_SEGM_SIZE;
	TEST_ASSERT_EQUAL_UINT32(expected, 10);
}

TEST_GROUP_RUNNER(get_blocks_num)
{
    RUN_TEST_CASE(get_blocks_num, 
		          then_blocks_number_should_be_calculated);
}





