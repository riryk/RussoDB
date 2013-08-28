#include "unity_fixture.h"
#include "hashtable.h"


TEST_GROUP(when_create_hashtable);


uint hashFuncTest  (void* key, ulong keySize) { return - 111; }
int hashCmpFuncTest (char* Key1, char* Key2, ulong keySize) { return -111; }

Hashtable tbl;



TEST_SETUP(when_create_hashtable)
{
    SHashtableSettings  set;
	set.hashFunc = hashFuncTest;
	set.hashCmp = hashCmpFuncTest;
	set.segmSize = 10;
	set.segmShift = 2;
	set.keyLen = 123;

	tbl = createHashtable(
		   "testTable123", 
		   0, 
		   &set, 
		   HASH_FUNC | HASH_CMP | HASH_SEG);
}

TEST_TEAR_DOWN(when_create_hashtable)
{
}

TEST(when_create_hashtable, then_tbl_is_not_null)
{
    TEST_ASSERT_NOT_NULL(tbl);
}

TEST(when_create_hashtable, then_hash_func_and_cmp_must_be_set)
{
	TEST_ASSERT_EQUAL_HEX(tbl->hashFunc, hashFuncTest);
	TEST_ASSERT_EQUAL_HEX(tbl->hashCmp, hashCmpFuncTest);
}

TEST(when_create_hashtable, then_segm_size_and_shift_must_be_set)
{
	TEST_ASSERT_EQUAL_UINT32(tbl->segmSize, 10);
	TEST_ASSERT_EQUAL_UINT32(tbl->segmShift, 2);
}

TEST(when_create_hashtable, then_key_len_must_be_set)
{
	TEST_ASSERT_EQUAL_UINT32(tbl->keyLen, 123);
}

TEST(when_create_hashtable, then_only_one_malloc_must_be_called)
{
    
}

TEST_GROUP_RUNNER(when_create_hashtable)
{
   RUN_TEST_CASE(when_create_hashtable, then_tbl_is_not_null);
   RUN_TEST_CASE(when_create_hashtable, then_hash_func_and_cmp_must_be_set);
   RUN_TEST_CASE(when_create_hashtable, then_segm_size_and_shift_must_be_set);
   RUN_TEST_CASE(when_create_hashtable, then_key_len_must_be_set);
   RUN_TEST_CASE(when_create_hashtable, then_only_one_malloc_must_be_called);
}


