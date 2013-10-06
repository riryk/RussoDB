#include "hashtable.h"
#include "unity_fixture.h"
#include "hashtablemanager.h"
#include "fakememmanager.h"
#include "hashtable_helper.h"

TEST_GROUP(expand_hashtable1);

int               currentKey;

Hashtable         tbl;
Hashtable         tblExtended;

IHashtableManager m_wet1;
IHashtableManager m_wet1_fakeCommon;

SListItemCount    originHashListItems[1000];
int*              originHashListItemsCount;

int fakeNextPowerOf2(long num)
{
	return (num == 10) ? 17 : num;
}

int fakeCalcSegmsNum(int numHashLists, ulong segmSize)
{
    return 5;
}

uint fakeCalcLowMask(int numHashLists)
{
    return 15;
}

uint fakeCalcHighMask(int numHashLists)
{
    return 31;
}

const SICommon sFakeCommonHelper = 
{ 
	fakeNextPowerOf2
};

const SIHashtableHelper sFakeHashtableHelper = 
{ 
    fakeCalcSegmsNum,
    fakeCalcLowMask,
    fakeCalcHighMask
};

IHashtableManager DoCreateTblManager()
{
    IHashtableManager man = (IHashtableManager)malloc(sizeof(SIHashtableManager));
    man->memManager       = &sFakeMemManager;
	man->commonHelper     = &sCommonHelper;
	man->hashtableHelper  = &sFakeHashtableHelper;
    man->createHashtable  = createHashtable;
    man->hashFind         = hashFind;
    man->hashInsert       = hashInsert;
    return man;
}

IHashtableManager DoCreateFakeTblManager()
{
    IHashtableManager man = DoCreateTblManager();
	man->commonHelper     = &sFakeCommonHelper;
	man->hashtableHelper  = &sHashtableHelper;
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
    m_wet1_fakeCommon = DoCreateTblManager();
}

GIVEN(expand_hashtable1)
{
	tbl         = DoCreateTable(m_wet1);
    backupMemory();

    tblExtended = DoCreateTable(m_wet1_fakeCommon);
	tblExtended->numHashLists = 17;
	tblExtended->nSegs = 5;

	backupMemory();
}

WHEN(expand_hashtable1) 
{
	int numToFill = tbl->hashListSize * tbl->numHashLists + 1;

	clearHashLists();
    currentKey = 0;
	fillTableToExpansion(
		tbl,
		m_wet1, 
		&currentKey, 
		numToFill);
	
	currentKey++;
    m_wet1->hashInsert(m_wet1, tbl, &currentKey);
    calculateHashListsLens(tbl, NULL);

    fillTableToExpansion(
		tbl,
		m_wet1, 
		&currentKey, 
		tbl->hashListSize - 1);

	copyHashLists(originHashListItems, &originHashListItemsCount);

	clearHashLists();
	currentKey = 0;
    fillTableToExpansion(
		tblExtended,
		m_wet1_fakeCommon, 
		&currentKey, 
		numToFill + 1 + tbl->hashListSize - 1);
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


