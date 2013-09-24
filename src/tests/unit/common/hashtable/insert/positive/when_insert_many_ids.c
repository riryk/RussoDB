#include "unity_fixture.h"
#include "hashtable.h"
#include "hashtablemanager.h"
#include "fakememmanager.h"

#define MAX_ALLOWED_DEVIATION (0.1)

typedef struct SListItemCount
{
	uint       listId;	    
	uint	   count;
	float      deviation;
} SListItemCount, *ListItemCount;

Hashtable          tbl;
IHashtableManager  m_wimi;
SListItemCount     hashListItemsCount[1000];
int                averageCount;

SETUP_DEPENDENCIES(hash_insert_many_ids) 
{
    m_wimi = (IHashtableManager)malloc(sizeof(SIHashtableManager));
    m_wimi->memManager = &sFakeMemManager;
    m_wimi->createHashtable = createHashtable;
	m_wimi->hashFind = hashFind;
	m_wimi->hashInsert = hashInsert;
}

GIVEN(hash_insert_many_ids) 
{
    SHashtableSettings  set;
    memset(&set, 0, sizeof(SHashtableSettings));
	set.hashFunc = getHashId;
	set.keyLen = sizeof(int);
	set.valLen = 34;
	set.hashListSize = 20;
	set.segmsAmount = DEFAULT_SEGS_AMOUNT;
    set.maxSegmsAmount = NO_SEGS_AMOUNT_RESTRICTIONS;
	set.segmSize = 4;
	set.segmShift = 2;

	tbl = m_wimi->createHashtable(
		        m_wimi,
		        "test", 
		        200, 
		        &set, 
		        HASH_FUNC | HASH_ITEM | HASH_LIST_SIZE | HASH_SEG);

	backupMemory();
}

WHEN(hash_insert_many_ids) 
{
    int i, max = 100000;
	for (i = 0; i < max; i++)
	{
		m_wimi->hashInsert(m_wimi, tbl, &i);
	}  
}

TEST_TEAR_DOWN(hash_insert_many_ids)
{
	m_wimi->memManager->freeAll();
	free(m_wimi);
}

int calculateListCount(HashItem listPtr)
{
	int count = 0;
    HashItem currPtr = listPtr;
	while (currPtr != NULL)
	{
        count++;
		currPtr = currPtr->next;
	}
	return count;
}

TEST(hash_insert_many_ids, then_items_must_be_uniformly_distributed)
{
	int            i, j, nSegs = tbl->lowMask / tbl->segmSize + 1;
	int            maxListNum  = tbl->lowMask;
	int            segSize     = tbl->segmSize;
    HashSegment    segm;
    HashItem       listPtr; 
	int            listId;
	int            totalCount = 0;
    ListItemCount  lCount;
	int            averageCount = tbl->numItems / (maxListNum + 1);

	TEST_ASSERT_NOT_NULL(tbl);

	for (i = 0; i < nSegs; i++)
	{
		segm = tbl->segments[i];
        for (j = 0; j < segSize; j++)
	    {
			listId  = segSize * i + j;
            listPtr = segm[j];
            lCount  = &hashListItemsCount[listId];
			lCount->listId = listId;
			lCount->count = calculateListCount(listPtr);
			lCount->deviation = ((float)abs(lCount->count - averageCount)) / ((float)abs(averageCount));

            TEST_ASSERT_FALSE(lCount->deviation > MAX_ALLOWED_DEVIATION);
	    }
	}
}

TEST_GROUP_RUNNER(hash_insert_many_ids)
{
	RUN_TEST_CASE(hash_insert_many_ids, then_items_must_be_uniformly_distributed);
}


