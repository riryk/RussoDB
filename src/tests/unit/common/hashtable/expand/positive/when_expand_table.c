#include "hashtable.h"
#include "unity_fixture.h"
#include "hashtablemanager.h"
#include "fakememmanager.h"


TEST_GROUP(expand_hashtable);

Hashtable         tbl;
IHashtableManager m_wet;
int               oldSegsNum; 
int               maxFillItems;
int               currentKey = 0;
int               insertedKey;
void*             foundItem;

SETUP_DEPENDENCIES(expand_hashtable) 
{
    m_wet = (IHashtableManager)malloc(sizeof(SIHashtableManager));
    m_wet->memManager = &sFakeMemManager;
    m_wet->createHashtable = createHashtable;
    m_wet->hashFind = hashFind;
    m_wet->hashInsert = hashInsert;
}

GIVEN(expand_hashtable)
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
	set.isWithoutExtention = False;

	tbl = m_wet->createHashtable(
		        m_wet,
		        "test", 
		        200, 
		        &set, 
		        HASH_FUNC | HASH_ITEM | HASH_LIST_SIZE | HASH_SEG | HASH_WITHOUT_EXTENTION);

	backupMemory();

	oldSegsNum = tbl->nSegs;
}

void DoFillEntireTable()
{
	int i;
    maxFillItems = tbl->hashListSize * tbl->numHashLists + 1;

	for (i = 0; i < maxFillItems; i++)
	{
		currentKey++;
		m_wet->hashInsert(m_wet, tbl, &currentKey);
		calculateHashListsLens(tbl, NULL);
	}
}

WHEN(expand_hashtable) 
{
    DoFillEntireTable();

	insertedKey = currentKey++;
    foundItem = m_wet->hashInsert(m_wet, tbl, &insertedKey);
    calculateHashListsLens(tbl, NULL);
}

TEST_TEAR_DOWN(expand_hashtable)
{
	m_wet->memManager->freeAll();
	free(m_wet);
}

TEST(expand_hashtable, then_tbl_is_not_null)
{
    TEST_ASSERT_NOT_NULL(tbl);
}

TEST(expand_hashtable, then_new_segment_must_be_allocated)
{
    int expectedMemLen = tbl->segmSize * sizeof(HashList);
    int newItemsAllocs = maxFillItems / tbl->numItemsToAlloc + 1;

	TEST_ASSERT_TRUE(fakeMemStorageCount > 0);
	TEST_ASSERT_EQUAL_UINT32(fakeMemStorage[newItemsAllocs].memLen, expectedMemLen);
}

TEST(expand_hashtable, then_the_inserted_item_must_be_found)
{
    foundItem = m_wet->hashFind(tbl, &insertedKey);

    TEST_ASSERT_NOT_NULL(foundItem);	
}

TEST_GROUP_RUNNER(expand_hashtable)
{
    RUN_TEST_CASE(expand_hashtable, then_tbl_is_not_null);
    RUN_TEST_CASE(expand_hashtable, then_new_segment_must_be_allocated);
    RUN_TEST_CASE(expand_hashtable, then_the_inserted_item_must_be_found);
}


