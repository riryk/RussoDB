#include "unity_fixture.h"
#include "hashtable.h"
#include "hashtablemanager.h"
#include "fakememmanager.h"

TEST_GROUP(insert_many_ids_into_same_list);

Hashtable tbl;
IHashtableManager m_wimisl;
void* foundItem;

uint wimisl_mockedHash(void* key, ulong keySize) { return 11111111; }

SETUP_DEPENDENCIES(insert_many_ids_into_same_list) 
{
    m_wimisl = (IHashtableManager)malloc(sizeof(SIHashtableManager));
    m_wimisl->memManager = &sFakeMemManager;
    m_wimisl->createHashtable = createHashtable;
	m_wimisl->hashLookUp = hashLookUp;
}

GIVEN(insert_many_ids_into_same_list) 
{
    SHashtableSettings  set;
    memset(&set, 0, sizeof(SHashtableSettings));
	set.hashFunc = wimisl_mockedHash;
	set.keyLen = sizeof(int);
	set.valLen = 34;

	tbl = m_wimisl->createHashtable(
		        m_wimisl,
		        "test", 
		        2000, 
		        &set, 
		        HASH_FUNC | HASH_ITEM);

	backupMemory();
}

WHEN(insert_many_ids_into_same_list) 
{
	int i, max = 1000;
	for (i = 0; i < max; i++)
	{
       m_wimisl->hashLookUp(m_wimisl, tbl, &i, HASH_INSERT);
	}
}

TEST_TEAR_DOWN(insert_many_ids_into_same_list)
{
	m_wimisl->memManager->freeAll();
	free(m_wimisl);
}

TEST(insert_many_ids_into_same_list, then_tbl_is_not_null)
{
    TEST_ASSERT_NOT_NULL(tbl);
}

TEST(insert_many_ids_into_same_list, then_hash_list_must_be_filled)
{
	int      listId   = 11111111 & tbl->lowMask;
	int      sNum     = listId >> tbl->segmShift; 
	int      sInd     = listId & (tbl->segmSize - 1);
	HashItem listPtr  = tbl->segments[sNum][sInd];
    int      numItems = 0;
	int      key      = -1;

    while (listPtr != NULL)
	{
        key = *(int*)GET_HASH_KEY(listPtr);
        TEST_ASSERT_EQUAL_UINT32(key, numItems++);
        listPtr = listPtr->next;
	}

    TEST_ASSERT_EQUAL_UINT32(numItems, 1000);
}

TEST(insert_many_ids_into_same_list, then_must_be_able_to_find_items)
{
    int      i;
    for (i = 0; i < 1000; i++)
	{
        foundItem =  m_wimisl->hashLookUp(m_wimisl, tbl, &i, HASH_FIND);
	}
}

TEST_GROUP_RUNNER(insert_many_ids_into_same_list)
{
    RUN_TEST_CASE(insert_many_ids_into_same_list, then_tbl_is_not_null);
    RUN_TEST_CASE(insert_many_ids_into_same_list, then_hash_list_must_be_filled);
    RUN_TEST_CASE(insert_many_ids_into_same_list, then_must_be_able_to_find_items);
}


