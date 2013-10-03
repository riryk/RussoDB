#include "unity_fixture.h"
#include "hashtable.h"
#include "hashtablemanager.h"
#include "fakememmanager.h"
#include "hashtable_helper.h"

Hashtable          tbl;
IHashtableManager  m_wimi;

SETUP_DEPENDENCIES(hash_insert_many_ids) 
{
    m_wimi = (IHashtableManager)malloc(sizeof(SIHashtableManager));
    m_wimi->memManager = &sFakeMemManager;
	m_wimi->commonHelper = &sCommonHelper;
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
	clearHashLists();
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

TEST(hash_insert_many_ids, then_items_must_be_uniformly_distributed)
{
    calculateHashListsLens(tbl, checkListCountDeviation);	
}

TEST_GROUP_RUNNER(hash_insert_many_ids)
{
	RUN_TEST_CASE(hash_insert_many_ids, then_items_must_be_uniformly_distributed);
}


