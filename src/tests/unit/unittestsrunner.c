#include "unittestsrunner.h"

void runAllUnitTests()
{
    RUN_TEST_GROUP(file_open_with_create_flag);
	RUN_TEST_GROUP(file_cache_delete);
	RUN_TEST_GROUP(file_cache_insert_multiple_times);
	RUN_TEST_GROUP(file_cache_insert);
	RUN_TEST_GROUP(file_cache_get_all_free_items);
	RUN_TEST_GROUP(file_cache_get_free_several_times);
	RUN_TEST_GROUP(file_cache_get_free);
	RUN_TEST_GROUP(file_cache_realloc);
    RUN_TEST_GROUP(compute_size_for_long_var_size_attr);
    RUN_TEST_GROUP(build_row_with_long_var_size_attr);
	RUN_TEST_GROUP(compute_size_for_shortable_var_size_attr);
    RUN_TEST_GROUP(build_row_with_shortable_var_size_attr);
	RUN_TEST_GROUP(compute_size_for_var_size_attr_with_the_first_bit_1);
    RUN_TEST_GROUP(build_row_with_var_size_attr_with_the_first_bit_1);
	RUN_TEST_GROUP(build_row_with_var_size_attr_with_the_first_byte_1);
	RUN_TEST_GROUP(compute_size_for_var_size_attr_with_the_first_byte_1);
    RUN_TEST_GROUP(build_row_with_fixed_size_attrs);
    RUN_TEST_GROUP(compute_size_for_row_with_fixed_size_attrs);
	RUN_TEST_GROUP(relrow_tests);
	RUN_TEST_GROUP(types_tests);
	RUN_TEST_GROUP(gen_hash);
	RUN_TEST_GROUP(create_hashtable);
    RUN_TEST_GROUP(hash_insert_id);
	RUN_TEST_GROUP(hash_insert_existed_id)
	RUN_TEST_GROUP(insert_many_ids_into_same_list);
	RUN_TEST_GROUP(expand_hashtable);
	RUN_TEST_GROUP(expand_hashtable1);
    RUN_TEST_GROUP(expand_hashtable2);
    RUN_TEST_GROUP(hash_insert_many_ids);
}