#include "unity_fixture.h"
#include "fakememmanager.h"
#include "filemanager.h"
#include "relfilemanager.h"
#include <io.h>
#include "file_manager_helper.h"
#include "snprintf.h"

TEST_GROUP(write_block);

IRelFileManager rfm_wf;
SRelData        rfm_rel;
FileSeg         rfm_seg;
int             rfm_fd;
char*           rfm_sstart = "test_result";
char*           rfm_buf;

SETUP_DEPENDENCIES(write_block) 
{
    rfm_wf = (IRelFileManager)malloc(sizeof(SIRelFileManager));
    rfm_wf->memManager         = &sFakeMemManager;
    rfm_wf->fileManager        = &sFileManager;
	rfm_wf->ctorRelFileMan     = ctorRelFileMan;
	rfm_wf->openRel            = openRel;
	rfm_wf->createRelPart      = createRelPart;
	rfm_wf->getFilePath        = getFilePath;
	rfm_wf->openRelSegm        = openRelSegm;
	rfm_wf->closeSegm          = closeSegm;
	rfm_wf->findBlockSegm      = findBlockSegm;
	rfm_wf->writeBlock         = writeBlock;
}

GIVEN(write_block) 
{
	rfm_wf->ctorRelFileMan(rfm_wf);

	rfm_rel.relKey.node.relId      = 1;
	rfm_rel.relKey.node.databaseId = 1;
	rfm_rel.relKey.node.tblSpaceId = GLOBAL_TBL_SPACE;
}

WHEN(write_block)
{
	char* somebuf = "some buffer";

   	rfm_buf = malloc(BLOCK_SIZE);

	strcpy(rfm_buf, somebuf);

	rfm_seg = rfm_wf->writeBlock(rfm_wf, rfm_sstart, &rfm_rel, FILE_PART_MAIN, 10, rfm_buf); 
	rfm_fd  = fileCache[rfm_seg->find].fileDesc;

    _commit(rfm_fd);
}

TEST_TEAR_DOWN(write_block)
{
	close(rfm_fd);
	remove("test_result/global/1");

	rfm_wf->memManager->freeAll();
	free(rfm_wf);
	free(rfm_buf);
}

TEST(write_block, then_the_block_should_be_written)
{	
	char* buffer  = malloc(BLOCK_SIZE);
	long  seekpos = lseek(rfm_fd, 10 * BLOCK_SIZE, 0);
	int   readres;

	TEST_ASSERT_EQUAL_UINT32(seekpos, 10 * BLOCK_SIZE);
    readres = read(rfm_fd, buffer, BLOCK_SIZE);

    TEST_ASSERT_EQUAL_STRING(rfm_buf, buffer);
}

TEST_GROUP_RUNNER(write_block)
{
    RUN_TEST_CASE(write_block, then_the_block_should_be_written);
}





