#include "hashtable.h"
#include "unity_fixture.h"
#include "hashtablemanager.h"
#include "fakememmanager.h"

TEST_GROUP(create_hashtable);


uint hashFuncTest  (void* key, ulong keySize) { return - 111; }
int hashCmpFuncTest (char* Key1, char* Key2, ulong keySize) { return -111; }


Hashtable tbl;
IHashtableManager m_wch;

SETUP_DEPENDENCIES(create_hashtable) 
{
   m_wch = (IHashtableManager)malloc(sizeof(SIHashtableManager));
   m_wch->memManager = &sFakeMemManager;
   m_wch->commonHelper = &sCommonHelper;
   m_wch->hashtableHelper = &sHashtableHelper;
   m_wch->createHashtable = createHashtable;
}

GIVEN(create_hashtable) { }

WHEN(create_hashtable)
{
	SHashtableSettings  set;
    memset(&set, 0, sizeof(SHashtableSettings));

	set.hashFunc = hashFuncTest;
	set.hashCmp = hashCmpFuncTest;
	set.segmSize = 10;
	set.segmShift = 2;
	set.keyLen = 123;
	set.valLen = 1034;
    set.hashListSize = 16;
    set.segmsAmount = 64;
    set.maxSegmsAmount = 128;

	tbl = m_wch->createHashtable(
		        m_wch,
		        "testTable123", 
		        900, 
		        &set, 
		        HASH_FUNC | HASH_CMP | HASH_SEG | HASH_ITEM | HASH_SEG | HASH_LIST_SIZE );
}

TEST_TEAR_DOWN(create_hashtable)
{
	m_wch->memManager->freeAll();
	free(m_wch);
}

TEST(create_hashtable, then_tbl_is_not_null)
{
    TEST_ASSERT_NOT_NULL(tbl);
}

TEST(create_hashtable, then_hash_func_and_cmp_must_be_set)
{
	TEST_ASSERT_EQUAL_HEX(tbl->hashFunc, hashFuncTest);
	TEST_ASSERT_EQUAL_HEX(tbl->hashCmp, hashCmpFuncTest);
}

TEST(create_hashtable, then_hash_key_len_and_val_len_must_be_set)
{
	TEST_ASSERT_EQUAL_HEX(tbl->keyLen, 123);
	TEST_ASSERT_EQUAL_HEX(tbl->valLen, 1034);
}

TEST(create_hashtable, then_segm_size_and_shift_must_be_set)
{
	TEST_ASSERT_EQUAL_UINT32(tbl->segmSize, 10);
	TEST_ASSERT_EQUAL_UINT32(tbl->segmShift, 2);
}

TEST(create_hashtable, then_hash_list_items_must_be_set)
{
	TEST_ASSERT_EQUAL_UINT32(tbl->segmsAmount, 64);
	TEST_ASSERT_EQUAL_UINT32(tbl->maxSegmsAmount, 128);
    TEST_ASSERT_EQUAL_UINT32(tbl->hashListSize, 16);
}

TEST(create_hashtable, then_mask_must_be_set)
{
	TEST_ASSERT_EQUAL_UINT32(tbl->lowMask, 63);
	TEST_ASSERT_EQUAL_UINT32(tbl->highMask, 127);
}

TEST(create_hashtable, then_items_to_alloc_must_be_set)
{
	TEST_ASSERT_EQUAL_UINT32(tbl->numItemsToAlloc, 55);
}

TEST(create_hashtable, then_name_must_be_set_only_with_one_malloc)
{
	uint expectedMemLen = sizeof(SHashtable) + strlen("testTable123") + 1;

	TEST_ASSERT_EQUAL_STRING(tbl->name, "testTable123");
	TEST_ASSERT_TRUE(fakeMemStorageCount > 0);
    
	TEST_ASSERT_EQUAL_UINT32(fakeMemStorage[0].memLen, expectedMemLen);
}

TEST(create_hashtable, then_segments_with_start_items_must_be_alloced)
{
	uint expSegsLen = 64 * sizeof(HashSegment);
    uint expListLen = 10 * sizeof(HashList);

	TEST_ASSERT_TRUE(fakeMemStorageCount > 1);
    TEST_ASSERT_EQUAL_UINT32(fakeMemStorage[1].memLen, expSegsLen);

    TEST_ASSERT_TRUE(fakeMemStorageCount >= 8);
    TEST_ASSERT_EQUAL_UINT32(fakeMemStorage[2].memLen, expListLen);
    TEST_ASSERT_EQUAL_UINT32(fakeMemStorage[3].memLen, expListLen);
	TEST_ASSERT_EQUAL_UINT32(fakeMemStorage[4].memLen, expListLen);
	TEST_ASSERT_EQUAL_UINT32(fakeMemStorage[5].memLen, expListLen);
	TEST_ASSERT_EQUAL_UINT32(fakeMemStorage[6].memLen, expListLen);
	TEST_ASSERT_EQUAL_UINT32(fakeMemStorage[7].memLen, expListLen);
    TEST_ASSERT_EQUAL_UINT32(fakeMemStorage[8].memLen, expListLen);
}

TEST_GROUP_RUNNER(create_hashtable)
{
    RUN_TEST_CASE(create_hashtable, then_tbl_is_not_null);
    RUN_TEST_CASE(create_hashtable, then_hash_func_and_cmp_must_be_set);
    RUN_TEST_CASE(create_hashtable, then_hash_key_len_and_val_len_must_be_set);
    RUN_TEST_CASE(create_hashtable, then_segm_size_and_shift_must_be_set);
    RUN_TEST_CASE(create_hashtable, then_hash_list_items_must_be_set);
    RUN_TEST_CASE(create_hashtable, then_mask_must_be_set);
    RUN_TEST_CASE(create_hashtable, then_items_to_alloc_must_be_set);

    RUN_TEST_CASE(create_hashtable, then_name_must_be_set_only_with_one_malloc);
    RUN_TEST_CASE(create_hashtable, then_segments_with_start_items_must_be_alloced);
}


