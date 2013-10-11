#include "hashtable.h"
#include "unity_fixture.h"
#include "hashtablemanager.h"
#include "fakememmanager.h"
#include "hashtable_helper.h"

TEST_GROUP(expand_hashtable3);

int               currentKey;

Hashtable         tbl;
IHashtableManager m_wet3;

SETUP_DEPENDENCIES(expand_hashtable3) 
{
    m_wet3    = (IHashtableManager)malloc(sizeof(SIHashtableManager));
    m_wet3->memManager          = &sFakeMemManager;
	m_wet3->commonHelper        = &sCommonHelper;
	m_wet3->hashtableHelper     = &sHashtableHelper;
    m_wet3->createHashtable     = createHashtable;
    m_wet3->hashFind            = hashFind;
    m_wet3->hashInsert          = hashInsert;
}

GIVEN(expand_hashtable3)
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

	tbl = m_wet3->createHashtable(
		        m_wet3,
		        "test", 
		        200, 
		        &set, 
		        HASH_FUNC | HASH_ITEM | HASH_LIST_SIZE | HASH_SEG | HASH_WITHOUT_EXTENTION);

    backupMemory();
}

WHEN(expand_hashtable3) 
{
	clearHashLists();
    currentKey = 0;
	fillTableToExpansion(
		tbl,
		m_wet3, 
		&currentKey, 
		2 * tbl->hashListSize * tbl->numHashLists);
}

TEST_TEAR_DOWN(expand_hashtable3)
{
	m_wet3->memManager->freeAll();
	free(m_wet3);
}

TEST(expand_hashtable3, then_a_new_memory_must_be_allocated)
{
    
}

TEST(expand_hashtable3, then_the_old_segments_memory_must_be_freed)
{
    
}

TEST(expand_hashtable3, then_the_segments_number_must_be_doubled)
{
    
}

TEST_GROUP_RUNNER(expand_hashtable3)
{
    RUN_TEST_CASE(expand_hashtable3, then_a_new_memory_must_be_allocated);
    RUN_TEST_CASE(expand_hashtable3, then_the_old_segments_memory_must_be_freed);
    RUN_TEST_CASE(expand_hashtable3, then_the_segments_number_must_be_doubled);
}


