#include "unity_fixture.h"
#include "hashtable.h"
#include "hashtablemanager.h"
#include "fakememmanager.h"
#include <windows.h>

TEST_GROUP(hash_insert_id);

int key;
Hashtable tbl;
HashItem insertedItem;
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
	insertedItem = m_wii->hashLookUp(
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
    uint expectedItemSize = ALIGN(sizeof(SHashItem)) + ALIGN(34) + ALIGN(sizeof(int));
	uint expectedBunchLen = expectedItemSize * 32;

	TEST_ASSERT_TRUE(fakeMemStorageCount > 0);
	TEST_ASSERT_EQUAL_UINT32(fakeMemStorage[0].memLen, expectedBunchLen);
}

TEST(hash_insert_id, then_new_items_must_be_added_to_free_list)
{
	HashItem curr, prev;
    int freeListCount = 0;
    uint expectedItemSize = ALIGN(sizeof(SHashItem)) + ALIGN(34) + ALIGN(sizeof(int));
    
    TEST_ASSERT_NOT_NULL(tbl->freeList);

	prev = NULL;
    curr = tbl->freeList;

	while (curr != NULL)
	{
		if (prev != NULL)
            TEST_ASSERT_EQUAL_UINT32((int)prev - (int)curr, expectedItemSize);

		freeListCount++;

        prev = curr;
		curr = curr->next;
	}
	TEST_ASSERT_EQUAL_UINT32(freeListCount, 32 - 1);
}

TEST(hash_insert_id, then_num_of_items_must_be_set)
{
    TEST_ASSERT_EQUAL_UINT32(tbl->numItems, 1);
}

TEST(hash_insert_id, then_hash_value_must_be_set_to_hash_item)
{
    uint expectedHash = tbl->hashFunc(&key, sizeof(int));

	TEST_ASSERT_NULL(insertedItem->next);
	TEST_ASSERT_EQUAL_UINT32(insertedItem->hash, expectedHash);
}

TEST(hash_insert_id, then_key_must_be_set_to_hash_item)
{
    TEST_ASSERT_NOT_NULL(insertedItem->key);
	TEST_ASSERT_EQUAL_UINT(1000, *(int*)insertedItem->key);
}

TEST(hash_insert_id, then_it_must_be_possible_to_write_value)
{
	char* testVal = "I am a test value";
    uint testValLen = strlen(testVal);

	TEST_ASSERT_NOT_NULL(insertedItem->value);	
    TEST_ASSERT_FALSE(IsBadCodePtr(insertedItem->value));

	tbl->hashCpy(insertedItem->value, testVal, testValLen);
}

TEST_GROUP_RUNNER(hash_insert_id)
{
    RUN_TEST_CASE(hash_insert_id, then_tbl_is_not_null);
    RUN_TEST_CASE(hash_insert_id, then_new_items_must_be_malloced);
	RUN_TEST_CASE(hash_insert_id, then_new_items_must_be_added_to_free_list);
    RUN_TEST_CASE(hash_insert_id, then_num_of_items_must_be_set);
	RUN_TEST_CASE(hash_insert_id, then_hash_value_must_be_set_to_hash_item);
    RUN_TEST_CASE(hash_insert_id, then_key_must_be_set_to_hash_item);
    RUN_TEST_CASE(hash_insert_id, then_it_must_be_possible_to_write_value);
}


