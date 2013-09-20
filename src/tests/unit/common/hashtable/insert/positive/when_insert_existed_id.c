#include "unity_fixture.h"
#include "hashtable.h"
#include "hashtablemanager.h"
#include "fakememmanager.h"
#include <windows.h>


TEST_GROUP(hash_insert_existed_id);


int                key = 1000;
Hashtable          tbl;
void*              insertedItem;
void*              insertedItem1;
IHashtableManager  m_wiei;


uint wiei_mockedHash(void* key, ulong keySize) { return 11111111; }


SETUP_DEPENDENCIES(hash_insert_existed_id) 
{
    m_wiei = (IHashtableManager)malloc(sizeof(SIHashtableManager));
    m_wiei->memManager = &sFakeMemManager;
    m_wiei->createHashtable = createHashtable;
	m_wiei->hashLookUp = hashLookUp;
}

GIVEN(hash_insert_existed_id) 
{
    SHashtableSettings  set;
    memset(&set, 0, sizeof(SHashtableSettings));
	set.hashFunc = wiei_mockedHash;
	set.keyLen = sizeof(int);
	set.valLen = 34;

	tbl = m_wiei->createHashtable(
		        m_wiei,
		        "test", 
		        2000, 
		        &set, 
		        HASH_FUNC | HASH_ITEM);
	
    insertedItem = m_wiei->hashLookUp(
		        m_wiei, 
				tbl, 
				&key, 
				HASH_INSERT);

	backupMemory();
}

WHEN(hash_insert_existed_id)
{
	insertedItem1 = m_wiei->hashLookUp(
		        m_wiei, 
				tbl, 
				&key, 
				HASH_INSERT);
}

TEST_TEAR_DOWN(hash_insert_existed_id)
{
	m_wiei->memManager->freeAll();
	free(m_wiei);
}

TEST(hash_insert_existed_id, then_tbl_is_not_null)
{
    TEST_ASSERT_NOT_NULL(tbl);
}

TEST(hash_insert_existed_id, then_existed_item_is_returned)
{
    int      key1     = *(int*)GET_HASH_KEY(insertedItem);
	int      key2     = *(int*)GET_HASH_KEY(insertedItem1);

	int      listId   = 11111111 & tbl->lowMask;
	int      sNum     = listId >> tbl->segmShift; 
	int      sInd     = listId & (tbl->segmSize - 1);
	HashItem listPtr  = tbl->segments[sNum][sInd];

	TEST_ASSERT_EQUAL_UINT32(key1, key2);
    TEST_ASSERT_NULL(listPtr->next);

    TEST_ASSERT_EQUAL_UINT32(*(int*)GET_HASH_KEY(listPtr), key);
}

TEST_GROUP_RUNNER(hash_insert_existed_id)
{
    RUN_TEST_CASE(hash_insert_existed_id, then_tbl_is_not_null);
    RUN_TEST_CASE(hash_insert_existed_id, then_existed_item_is_returned);
}


