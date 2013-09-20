#include "unity_fixture.h"
#include "hashtable.h"
#include "hashtablemanager.h"
#include "fakememmanager.h"


Hashtable tbl;
IHashtableManager m_wimi;

SETUP_DEPENDENCIES(hash_insert_many_ids) 
{
    m_wimi = (IHashtableManager)malloc(sizeof(SIHashtableManager));
    m_wimi->memManager = &sFakeMemManager;
    m_wimi->createHashtable = createHashtable;
	m_wimi->hashLookUp = hashLookUp;
}

GIVEN(hash_insert_many_ids) 
{
    SHashtableSettings  set;
    memset(&set, 0, sizeof(SHashtableSettings));
	set.hashFunc = getHashId;
	set.keyLen = sizeof(int);
	set.valLen = 34;

	tbl = m_wimi->createHashtable(
		        m_wimi,
		        "test", 
		        2000, 
		        &set, 
		        HASH_FUNC | HASH_ITEM);

	backupMemory();
}

WHEN(hash_insert_many_ids) 
{
    
}

TEST_TEAR_DOWN(hash_insert_many_ids)
{
	m_wimi->memManager->freeAll();
	free(m_wimi);
}

TEST(hash_insert_many_ids, then_tbl_is_not_null)
{
    TEST_ASSERT_NOT_NULL(tbl);
}

TEST(hash_insert_many_ids, then_items_must_be_uniformly_distributed)
{
    TEST_ASSERT_NOT_NULL(tbl);
}

TEST_GROUP_RUNNER(hash_insert_many_ids)
{
    RUN_TEST_CASE(hash_insert_many_ids, then_tbl_is_not_null);
	RUN_TEST_CASE(hash_insert_many_ids, then_items_must_be_uniformly_distributed);
}


