#include "hashtable.h"
#include "unity_fixture.h"
#include "hashtablemanager.h"
#include "fakememmanager.h"
#include "hashtable_helper.h"

TEST_GROUP(expand_hashtable2);

int               currentKey;

Hashtable         tbl;
IHashtableManager m_wet2;

SETUP_DEPENDENCIES(expand_hashtable2) 
{
    m_wet2    = (IHashtableManager)malloc(sizeof(SIHashtableManager));
    m_wet2->memManager          = &sFakeMemManager;
	m_wet2->commonHelper        = &sCommonHelper;
	m_wet2->hashtableHelper     = &sHashtableHelper;
    m_wet2->createHashtable     = createHashtable;
    m_wet2->hashFind            = hashFind;
    m_wet2->hashInsert          = hashInsert;
}

GIVEN(expand_hashtable2)
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

	tbl = m_wet2->createHashtable(
		        m_wet2,
		        "test", 
		        200, 
		        &set, 
		        HASH_FUNC | HASH_ITEM | HASH_LIST_SIZE | HASH_SEG | HASH_WITHOUT_EXTENTION);

    backupMemory();
}

WHEN(expand_hashtable2) 
{
	clearHashLists();
    currentKey = 0;
	fillTableToExpansion(
		tbl,
		m_wet2, 
		&currentKey, 
		2 * tbl->hashListSize * tbl->numHashLists);
}

TEST_TEAR_DOWN(expand_hashtable2)
{
	m_wet2->memManager->freeAll();
	free(m_wet2);
}

TEST(expand_hashtable2, then_the_high_and_low_masks_must_be_reinitialized)
{
	TEST_ASSERT_EQUAL_UINT32(tbl->lowMask, 31);  
	TEST_ASSERT_EQUAL_UINT32(tbl->highMask, 63);
}

TEST_GROUP_RUNNER(expand_hashtable2)
{
    RUN_TEST_CASE(expand_hashtable2, then_the_high_and_low_masks_must_be_reinitialized);
}


