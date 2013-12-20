#include "unity_fixture.h"
#include "fakememmanager.h"
#include "buffermanager.h"
#include "relfilemanager.h"
#include "latchmanager.h"

TEST_GROUP(get_buffer_from_ring);

IBufferManager bm_gbfr;
BufferInfo     b_gbfr;

SETUP_DEPENDENCIES(get_buffer_from_ring) 
{
    bm_gbfr = (IBufferManager)malloc(sizeof(SIBufferManager));
	bm_gbfr->relFileManager         = &sRelFileManager;
	bm_gbfr->hashtableManager       = &sHashtableManager;
	bm_gbfr->latchManager           = &sLatchManager;
	bm_gbfr->memoryManager          = &sTrackMemManager;

	bm_gbfr->ctorBufMan             = ctorBufMan;
	bm_gbfr->getBufferFromRingArray = getBufferFromRingArray;
	bm_gbfr->getBufferFromRing      = getBufferFromRing;
}

GIVEN(get_buffer_from_ring) 
{
	bm_gbfr->ctorBufMan(bm_gbfr);
}

WHEN(get_buffer_from_ring)
{
	b_gbfr = bm_gbfr->getBufferFromRing(bm_gbfr, NULL);
}

TEST_TEAR_DOWN(get_buffer_from_ring)
{
	bm_gbfr->relFileManager->memManager->freeAll();

	free(bm_gbfr);
}

TEST(get_buffer_from_ring, then_a_buffer_should_be_returned)
{	
    TEST_ASSERT_NOT_NULL(b_gbfr);	
}

TEST_GROUP_RUNNER(get_buffer_from_ring)
{
    RUN_TEST_CASE(get_buffer_from_ring, then_a_buffer_should_be_returned);
}





