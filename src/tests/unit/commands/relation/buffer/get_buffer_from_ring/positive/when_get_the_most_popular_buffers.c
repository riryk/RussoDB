#include "unity_fixture.h"
#include "fakememmanager.h"
#include "buffermanager.h"
#include "relfilemanager.h"
#include "latchmanager.h"

TEST_GROUP(get_the_most_popular_buffers);

IBufferManager  bm_gtmpb; 

BufferInfo      b_gtmpb;
BufferInfo      b_last_gtmpb;
BufferInfo      b_before_last_gtmpb;

SETUP_DEPENDENCIES(get_the_most_popular_buffers) 
{
    bm_gtmpb = (IBufferManager)malloc(sizeof(SIBufferManager));
	bm_gtmpb->relFileManager         = &sRelFileManager;
	bm_gtmpb->hashtableManager       = &sHashtableManager;
	bm_gtmpb->latchManager           = &sLatchManager;
	bm_gtmpb->memoryManager          = &sTrackMemManager;

	bm_gtmpb->ctorBufMan             = ctorBufMan;
	bm_gtmpb->getBufferFromRingArray = getBufferFromRingArray;
	bm_gtmpb->getBufferFromRing      = getBufferFromRing;
	bm_gtmpb->pinBuffer              = pinBuffer;
	bm_gtmpb->unpinBuffer            = unpinBuffer;
}

GIVEN(get_the_most_popular_buffers) 
{
	int i;

	bm_gtmpb->ctorBufMan(bm_gtmpb);
    
	for (i = 0; i < 5; i++)
	{
	    bm_gtmpb->pinBuffer(bm_gtmpb, &(bufInfos[0]), NULL);
		bm_gtmpb->unpinBuffer(bm_gtmpb, &(bufInfos[0]));
	}

	for (i = 0; i < 3; i++)
	{
        bm_gtmpb->pinBuffer(bm_gtmpb, &(bufInfos[1]), NULL);
        bm_gtmpb->unpinBuffer(bm_gtmpb, &(bufInfos[1]));
	}

	freeBufferState->first = FREENEXT_END_OF_LIST;
}

WHEN(get_the_most_popular_buffers)
{
	int i;

	for (i = 0; i < 1000 - 2; i++)
	{
	    b_gtmpb = bm_gtmpb->getBufferFromRing(bm_gtmpb, NULL);
        bm_gtmpb->pinBuffer(bm_gtmpb, b_gtmpb, NULL);
	}

	b_before_last_gtmpb = bm_gtmpb->getBufferFromRing(bm_gtmpb, NULL);
    bm_gtmpb->pinBuffer(bm_gtmpb, b_before_last_gtmpb, NULL);

	b_last_gtmpb = bm_gtmpb->getBufferFromRing(bm_gtmpb, NULL);; 
    bm_gtmpb->pinBuffer(bm_gtmpb, b_last_gtmpb, NULL);
}

TEST_TEAR_DOWN(get_the_most_popular_buffers)
{
	bm_gtmpb->relFileManager->memManager->freeAll();

	free(bm_gtmpb);
}

TEST(get_the_most_popular_buffers, then_more_popular_buffers_are_taken_last)
{	
    TEST_ASSERT_NOT_NULL(b_before_last_gtmpb);	
    TEST_ASSERT_NOT_NULL(b_last_gtmpb);	
	 

	TEST_ASSERT_EQUAL_INT(b_before_last_gtmpb->ind, 1);
    TEST_ASSERT_EQUAL_INT(b_last_gtmpb->ind, 0);
}

TEST_GROUP_RUNNER(get_the_most_popular_buffers)
{
    RUN_TEST_CASE(get_the_most_popular_buffers, then_more_popular_buffers_are_taken_last);
}





