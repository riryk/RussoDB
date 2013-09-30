#include "hashtable.h"
#include "unity_fixture.h"
#include "hashtablemanager.h"
#include "fakememmanager.h"


TEST_GROUP(expand_hashtable);

Hashtable tbl;
IHashtableManager m_wet;

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
}

WHEN(expand_hashtable) 
{
    int i;
	int maxFillItems = tbl->hashListSize * tbl->numHashLists;
    for (i = 0; i <= maxFillItems + 1; i++)
	{
		m_wet->hashInsert(m_wet, tbl, &i);
	}  
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

TEST_GROUP_RUNNER(expand_hashtable)
{
    RUN_TEST_CASE(expand_hashtable, then_tbl_is_not_null);
}


