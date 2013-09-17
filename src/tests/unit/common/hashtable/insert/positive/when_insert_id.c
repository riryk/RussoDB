#include "unity_fixture.h"
#include "hashtable.h"
#include "hashtablemanager.h"
#include "fakememmanager.h"


TEST_GROUP(hash_insert_id);

int key;
Hashtable tbl;
IHashtableManager m_wii;

SETUP_DEPENDENCIES(hash_insert_id) 
{
    m_wii = (IHashtableManager)malloc(sizeof(SIHashtableManager));
    m_wii->memManager = &sFakeMemManager;
    m_wii->createHashtable = createHashtable;
	m_wii->hashLookUp = hashLookUp;
}

GIVEN(hash_insert_id) 
{
    SHashtableSettings  set;
    memset(&set, 0, sizeof(SHashtableSettings));
	set.hashFunc = getHashId;
	set.keyLen = sizeof(int);
	set.valLen = 34;

	tbl = m_wii->createHashtable(
		        m_wii,
		        "test", 
		        2000, 
		        &set, 
		        HASH_FUNC | HASH_ITEM);
	key = 1000;
	backupMemory();
}

WHEN(hash_insert_id)
{
	m_wii->hashLookUp(
		        m_wii,
                tbl,
                &key,
				HASH_INSERT);
}

TEST_TEAR_DOWN(hash_insert_id)
{
	m_wii->memManager->freeAll();
	free(m_wii);
}

TEST(hash_insert_id, then_tbl_is_not_null)
{
    TEST_ASSERT_NOT_NULL(tbl);
}

TEST(hash_insert_id, then_new_items_must_be_malloced)
{
	uint expectedBunchLen = (8 + 40) * 36;

	TEST_ASSERT_TRUE(fakeMemStorageCount > 0);
	TEST_ASSERT_EQUAL_UINT32(fakeMemStorage[0].memLen, expectedBunchLen);
}

TEST(hash_insert_id, then_new_items_must_be_added_to_free_list)
{
	HashItem curr, prev;
    int freeListCount = 0;
    uint expectedItemSize = ALIGN(sizeof(SHashItem)) + 40;
    
    TEST_ASSERT_NOT_NULL(tbl->freeList);

	prev = NULL;
    curr = tbl->freeList;
    freeListCount++;

	while (curr != NULL)
	{
		if (prev != NULL)
            TEST_ASSERT_EQUAL_UINT32((int)prev - (int)curr, expectedItemSize);

        prev = curr;
		curr = curr->next;
		freeListCount++;
	}
	TEST_ASSERT_EQUAL_UINT32(freeListCount, 36);
}

TEST_GROUP_RUNNER(hash_insert_id)
{
    RUN_TEST_CASE(hash_insert_id, then_tbl_is_not_null);
    RUN_TEST_CASE(hash_insert_id, then_new_items_must_be_malloced);
	RUN_TEST_CASE(hash_insert_id, then_new_items_must_be_added_to_free_list)
}


