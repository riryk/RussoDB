#include "unittestsrunner.h"

void runAllUnitTests()
{
	RUN_TEST_GROUP(when_mix_jerkins);
	RUN_TEST_GROUP(when_gen_hash);
	RUN_TEST_GROUP(when_create_hashtable);
}