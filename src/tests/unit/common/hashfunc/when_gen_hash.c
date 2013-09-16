#include "unity_fixture.h"
#include "hashfunctions.h"


TEST_GROUP(gen_hash);

uint generatedHash1;
uint generatedHash2;
uint generatedHash3;
uint generatedHash4;

SETUP_DEPENDENCIES(gen_hash) { }

GIVEN(gen_hash) { }

WHEN(gen_hash)
{
    char* key = "test";
	generatedHash1 = getHashFast("test", 4);
    generatedHash2 = getHashFast("@", 1);
    generatedHash3 = getHashFast("I am a very very long string.", 30);
    generatedHash4 = getHashFast("I am @ very very long string.", 30);
}

TEST_TEAR_DOWN(gen_hash) { }

TEST(gen_hash, then_hash_should_be_generated)
{
    TEST_ASSERT_EQUAL_UINT32(generatedHash1, 1771415073);
	TEST_ASSERT_EQUAL_UINT32(generatedHash2, 1973689532);
	TEST_ASSERT_EQUAL_UINT32(generatedHash3, 3425969641);
	TEST_ASSERT_EQUAL_UINT32(generatedHash4, 422128119);
}

TEST_GROUP_RUNNER(gen_hash)
{
    RUN_TEST_CASE(gen_hash, then_hash_should_be_generated);
}


