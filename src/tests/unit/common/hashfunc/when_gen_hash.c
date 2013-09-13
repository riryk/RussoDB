#include "unity_fixture.h"
#include "hashfunctions.h"


TEST_GROUP(when_gen_hash);

uint generatedHash1;
uint generatedHash2;
uint generatedHash3;
uint generatedHash4;

TEST_SETUP(when_gen_hash)
{
	char* key = "test";
	generatedHash1 = getHashFast("test", 4);
    generatedHash2 = getHashFast("@", 1);
    generatedHash3 = getHashFast("I am a very very long string.", 30);
    generatedHash4 = getHashFast("I am @ very very long string.", 30);
}

TEST_TEAR_DOWN(when_gen_hash)
{
	
}

TEST(when_gen_hash, then_hash_should_be_generated)
{
    
}

TEST_GROUP_RUNNER(when_gen_hash)
{
    RUN_TEST_CASE(when_gen_hash, then_hash_should_be_generated);
}


