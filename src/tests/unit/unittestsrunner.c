#include "unittestsrunner.h"

void runAllUnitTests()
{
	RUN_TEST_GROUP(gen_hash);
	RUN_TEST_GROUP(create_hashtable);
    RUN_TEST_GROUP(hash_insert_id);
	RUN_TEST_GROUP(insert_many_ids_into_same_list);
}