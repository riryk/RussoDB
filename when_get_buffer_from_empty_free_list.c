#include "unity_fixture.h"
#include "fakememmanager.h"
#include "buffermanager.h"
#include "relfilemanager.h"
#include "latchmanager.h"

TEST_GROUP(get_buffer_from_empty_free_list);

IBufferManager  bm_gbfefl; 
BufferInfo      b_gbfefl;

SETUP_DEPENDENCIES(get_buffer_from_empty_free_list) 
{
    bm_gbfefl = (IBufferManager)malloc(sizeof(SIBufferManager));
	bm_gbfefl->relFileManager         = &sRelFileManager;
	bm_gbfefl->hashtableManager       = &sHashtableManager;
	bm_gbfefl->latchManager           = &sLatchManager;
	bm_gbfefl->memoryManager          = &sTrackMemManager;

	bm_gbfefl->ctorBufMan             = ctorBufMan;
	bm_gbfefl->getBufferFromRingArray = getBufferFromRingArray;
	bm_gbfefl->getBufferFromRing      = getBufferFromRing;
}

GIVEN(get_buffer_from_empty_free_list) 
{
	bm_gbfefl->ctorBufMan(bm_gbfefl);

	freeBufferState->first = FREENEXT_END_OF_LIST;
}

WHEN(get_buffer_from_empty_free_list)
{
	b_gbfefl = bm_gbfefl->getBufferFromRing(bm_gbfefl, NULL);
}

TEST_TEAR_DOWN(get_buffer_from_empty_free_list)
{
	bm_gbfefl->relFileManager->memManager->freeAll();

	free(bm_gbfefl);
}

TEST(get_buffer_from_empty_free_list, then_a_buffer_should_be_returned)
{	
    TEST_ASSERT_NOT_NULL(b_gbfefl);	
}

TEST_GROUP_RUNNER(get_buffer_from_empty_free_list)
{
    RUN_TEST_CASE(get_buffer_from_empty_free_list, then_a_buffer_should_be_returned);
}





