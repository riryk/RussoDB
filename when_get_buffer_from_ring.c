#include "unity_fixture.h"
#include "fakememmanager.h"
#include "buffermanager.h"
#include "relfilemanager.h"
#include "latchmanager.h"

TEST_GROUP(get_buffer_from_ring);

IBufferManager bm_gbfr;
BufRing        br_gbfr; 

SETUP_DEPENDENCIES(get_buffer_from_ring) 
{
    bm_gbfr = (IBufferManager)malloc(sizeof(SIBufferManager));
	bm_gbfr->relFileManager         = &sRelFileManager;
	bm_gbfr->hashtableManager       = &sHashtableHelper;
	bm_gbfr->latchManager           = &sLatchManager;

	bm_gbfr->ctorBufMan             = ctorBufMan;
	bm_gbfr->getBufferFromRingArray = getBufferFromRingArray;
	bm_gbfr->getBufferFromRing      = getBufferFromRing;

    br_gbfr = (BufRing)malloc(sizeof(SBufRing));
}

GIVEN(get_buffer_from_ring) 
{
	bm_gbfr->ctorBufMan(bm_gbfr);
}

WHEN(get_buffer_from_ring)
{
	bm_gbfr->getBufferFromRing(bm_gbfr, br_gbfr);
}

TEST_TEAR_DOWN(get_buffer_from_ring)
{
	bm_gbfr->relFileManager->memManager->freeAll();

	free(bm_gbfr);
	free(br_gbfr);
}

TEST(get_buffer_from_ring, then_test)
{	
	
}

TEST_GROUP_RUNNER(get_buffer_from_ring)
{
    RUN_TEST_CASE(get_buffer_from_ring, then_test);
}





