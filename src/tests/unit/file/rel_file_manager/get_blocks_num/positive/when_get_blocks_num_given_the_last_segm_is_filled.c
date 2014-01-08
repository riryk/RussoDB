#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"
#include "relfilemanager.h"
#include <io.h>
#include "file_manager_helper.h"
#include "snprintf.h"

TEST_GROUP(get_blocks_num_given_the_last_segm_is_filled);

IRelFileManager gbngtlsif_or;
SRelData        gbngtlsif_rel;
char*           gbngtlsif_start = "test_result";
int             gbngtlsif_blnum;
int             gbngtlsif_segm_size = 10;

FileSeg         gbngtlsif_part0;
FileSeg         gbngtlsif_part1;
FileSeg         gbngtlsif_part2; 
FileSeg         gbngtlsif_part3;
FileSeg         gbngtlsif_part;

SETUP_DEPENDENCIES(get_blocks_num_given_the_last_segm_is_filled) 
{
    gbngtlsif_or = (IRelFileManager)malloc(sizeof(SIRelFileManager));
    gbngtlsif_or->memManager         = &sFakeMemManager;
    gbngtlsif_or->fileManager        = &sFileManager;
	gbngtlsif_or->ctorRelFileMan     = ctorRelFileMan;
	gbngtlsif_or->openRel            = openRel;
	gbngtlsif_or->createRelPart      = createRelPart;
	gbngtlsif_or->getFilePath        = getFilePath;
	gbngtlsif_or->openRelSegm        = openRelSegm;
	gbngtlsif_or->closeSegm          = closeSegm;
}

FileSeg gbngtlsif_open_rel_segm(int i)
{
    return gbngtlsif_or->openRelSegm(
		               gbngtlsif_or, 
		               gbngtlsif_start, 
					   &gbngtlsif_rel, 
					   0, 
					   i, 
					   _O_CREAT);
}

GIVEN(get_blocks_num_given_the_last_segm_is_filled) 
{
	int fd, i;

	gbngtlsif_or->ctorRelFileMan(gbngtlsif_or);

	gbngtlsif_rel.relKey.node.relId      = 1;
	gbngtlsif_rel.relKey.node.databaseId = 1;
	gbngtlsif_rel.relKey.node.tblSpaceId = GLOBAL_TBL_SPACE;

	gbngtlsif_part0 = gbngtlsif_open_rel_segm(0);
    gbngtlsif_part1 = gbngtlsif_open_rel_segm(1);
	gbngtlsif_part2 = gbngtlsif_open_rel_segm(2);
	gbngtlsif_part3 = gbngtlsif_open_rel_segm(3);
    
	fd = fileCache[gbngtlsif_part3->find].fileDesc;

	for (i = 0; i < 10 * (1 << 13); i++)
        write(fd, "R", 1);

    _commit(fd);

	gbngtlsif_part0->next = gbngtlsif_part1;
    gbngtlsif_part1->next = gbngtlsif_part2;
    gbngtlsif_part2->next = gbngtlsif_part3;
	gbngtlsif_part3->next = NULL;

	gbngtlsif_rel.parts[0] = gbngtlsif_part0;
}

WHEN(get_blocks_num_given_the_last_segm_is_filled)
{
	gbngtlsif_blnum = getBlocksNum(
        gbngtlsif_or,
        gbngtlsif_start,
	    &gbngtlsif_rel,
	    0,
		gbngtlsif_segm_size);
}

TEST_TEAR_DOWN(get_blocks_num_given_the_last_segm_is_filled)
{
	gbngtlsif_or->closeSegm(gbngtlsif_or, gbngtlsif_part0);
	remove("test_result/global/1");

    gbngtlsif_or->closeSegm(gbngtlsif_or, gbngtlsif_part1);
	remove("test_result/global/1.1");

    gbngtlsif_or->closeSegm(gbngtlsif_or, gbngtlsif_part2);
	remove("test_result/global/1.2");

    gbngtlsif_or->closeSegm(gbngtlsif_or, gbngtlsif_part3);
	remove("test_result/global/1.3");

    gbngtlsif_or->closeSegm(gbngtlsif_or, gbngtlsif_part);
	remove("test_result/global/1.4");

	gbngtlsif_or->memManager->freeAll();
	free(gbngtlsif_or);
}

TEST(get_blocks_num_given_the_last_segm_is_filled, 
	 then_a_new_segment_should_be_added)
{	
	int     snum   = 1;
	gbngtlsif_part = gbngtlsif_rel.parts[0];

	while (gbngtlsif_part->next != NULL)
	{
		snum++;
	    gbngtlsif_part = gbngtlsif_part->next;
	}

	TEST_ASSERT_EQUAL_UINT32(snum, 5);
}

TEST_GROUP_RUNNER(get_blocks_num_given_the_last_segm_is_filled)
{
    RUN_TEST_CASE(get_blocks_num_given_the_last_segm_is_filled, 
		          then_a_new_segment_should_be_added);
}





