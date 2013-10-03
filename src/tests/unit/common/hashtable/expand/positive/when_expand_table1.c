#include "hashtable.h"
#include "unity_fixture.h"
#include "hashtablemanager.h"
#include "fakememmanager.h"


TEST_GROUP(expand_hashtable1);

Hashtable         tbl;
Hashtable         tblExtended;

IHashtableManager m_wet1;
IHashtableManager m_wet1_fakeCommon;

int fakeNextPowerOf2(long num)
{
	switch (num)
	{
	case 16:
		return 17;
	default:
        return num;
	}
}

IHashtableManager DoCreateTblManager()
{
    IHashtableManager man = (IHashtableManager)malloc(sizeof(SIHashtableManager));
    man->memManager = &sFakeMemManager;
	man->commonHelper = &sCommonHelper;
    man->createHashtable = createHashtable;
    man->hashFind = hashFind;
    man->hashInsert = hashInsert;
    return man;
}

IHashtableManager DoCreateFakeTblManager()
{
    IHashtableManager man = DoCreateTblManager();
	man->commonHelper = &fakeNextPowerOf2;
	return man;
}

Hashtable DoCreateTable(IHashtableManager man)
{
	Hashtable           tbl;

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

	tbl = man->createHashtable(
		        man,
		        "test", 
		        200, 
		        &set, 
		        HASH_FUNC | HASH_ITEM | HASH_LIST_SIZE | HASH_SEG | HASH_WITHOUT_EXTENTION);

	return tbl;
}

SETUP_DEPENDENCIES(expand_hashtable1) 
{
    m_wet1            = DoCreateTblManager();
    m_wet1_fakeCommon = DoCreateFakeTblManager();
}

GIVEN(expand_hashtable1)
{
	tbl         = DoCreateTable(m_wet1);
    backupMemory();

    tblExtended = DoCreateTable(m_wet1_fakeCommon);
	backupMemory();
}

WHEN(expand_hashtable1) 
{
	int numToFill = tbl->hashListSize * tbl->numHashLists + 1;

	fillTableToExpansion(
		tbl,
		m_wet, 
		&currentKey, 
		numToFill);
	
	currentKey++;
    m_wet->hashInsert(m_wet, tbl, &currentKey);
    calculateHashListsLens(tbl, NULL);

    fillTableToExpansion(
		tblExtended,
		m_wet, 
		&currentKey, 
		numToFill);
}

TEST_TEAR_DOWN(expand_hashtable1)
{
	m_wet1->memManager->freeAll();
	free(m_wet1);

	m_wet1_fakeCommon->memManager->freeAll();
	free(m_wet1_fakeCommon);
}

TEST(expand_hashtable1, then_the_items_distribution_must_be_the_same_as_without_extension)
{
   
}

TEST_GROUP_RUNNER(expand_hashtable1)
{
    RUN_TEST_CASE(expand_hashtable1, then_the_items_distribution_must_be_the_same_as_without_extension);
}


