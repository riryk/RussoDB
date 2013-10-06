#include "unity_fixture.h"
#include "hashtable.h"
#include "hashtablemanager.h"
#include "fakememmanager.h"
#include <windows.h>

TEST_GROUP(hash_insert_id);

int key;
Hashtable tbl;
void* insertedItem;
void* foundItem;
IHashtableManager m_wii;

uint wii_mockedHash(void* key, ulong keySize) { return 11111111; }

SETUP_DEPENDENCIES(hash_insert_id) 
{
    m_wii = (IHashtableManager)malloc(sizeof(SIHashtableManager));
    m_wii->memManager = &sFakeMemManager;
	m_wii->commonHelper = &sCommonHelper;
	m_wii->hashtableHelper = &sHashtableHelper;
    m_wii->createHashtable = createHashtable;
	m_wii->hashFind = hashFind;
	m_wii->hashInsert = hashInsert;
}

GIVEN(hash_insert_id) 
{
    SHashtableSettings  set;
    memset(&set, 0, sizeof(SHashtableSettings));
	set.hashFunc = wii_mockedHash;
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
	insertedItem = m_wii->hashInsert(m_wii, tbl, &key);
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
	uint expectedBunchLen = expectedItemSize * 36;

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
	TEST_ASSERT_EQUAL_UINT32(freeListCount, 36 - 1);
}

TEST(hash_insert_id, then_num_of_items_must_be_set)
{
    TEST_ASSERT_EQUAL_UINT32(tbl->numItems, 1);
}

TEST(hash_insert_id, then_key_must_be_set_to_hash_item)
{
    TEST_ASSERT_NOT_NULL(insertedItem);
	TEST_ASSERT_EQUAL_UINT(1000, *(int*)insertedItem);
}

TEST(hash_insert_id, then_it_must_be_possible_to_write_value)
{
	char* testVal = "I am a test value";
    uint testValLen = strlen(testVal);
    void* val = GET_HASH_VALUE(insertedItem, tbl->keyLen);

	TEST_ASSERT_NOT_NULL(val);	
    TEST_ASSERT_FALSE(IsBadCodePtr(val));

	tbl->hashCpy(val, testVal, testValLen);
}

TEST(hash_insert_id, then_the_item_must_be_found)
{
    foundItem = m_wii->hashFind(tbl, &key);

    TEST_ASSERT_NOT_NULL(foundItem);	
}

TEST_GROUP_RUNNER(hash_insert_id)
{
    RUN_TEST_CASE(hash_insert_id, then_tbl_is_not_null);
    RUN_TEST_CASE(hash_insert_id, then_new_items_must_be_malloced);
	RUN_TEST_CASE(hash_insert_id, then_new_items_must_be_added_to_free_list);
    RUN_TEST_CASE(hash_insert_id, then_num_of_items_must_be_set);
    RUN_TEST_CASE(hash_insert_id, then_key_must_be_set_to_hash_item);
    RUN_TEST_CASE(hash_insert_id, then_it_must_be_possible_to_write_value);
    RUN_TEST_CASE(hash_insert_id, then_the_item_must_be_found);
}


