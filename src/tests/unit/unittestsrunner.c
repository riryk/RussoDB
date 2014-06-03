#include "unittestsrunner.h"

void runAllUnitTests()
{
    RUN_TEST_GROUP(send_signal_to_another_process);
	RUN_TEST_GROUP(signal_thread_waiting_for_semaphore);
	RUN_TEST_GROUP(signal_main);
	RUN_TEST_GROUP(spin_lock_acquire_in_another_process_sem);
    RUN_TEST_GROUP(spin_lock_manager_acquire_without_contention_sem);
	RUN_TEST_GROUP(spin_lock_for_short_time_inter_process);
	RUN_TEST_GROUP(shared_mem_allocate_in_another_process);
	RUN_TEST_GROUP(shared_mem_create_in_another_process);
	RUN_TEST_GROUP(shared_mem_allocate);
	RUN_TEST_GROUP(shared_mem_create_several_times);
	RUN_TEST_GROUP(shared_mem_create);
	RUN_TEST_GROUP(spin_short_locks_are_applied_sequentially);
	RUN_TEST_GROUP(spin_locks_are_applied_sequentially);
	RUN_TEST_GROUP(spin_lock_for_short_time);
	RUN_TEST_GROUP(spin_lock_manager_acquire_without_contention);
	RUN_TEST_GROUP(sub_proc_start);
    RUN_TEST_GROUP(logger_start);
	RUN_TEST_GROUP(allocate_small_large_chunk);
	RUN_TEST_GROUP(free_large_chunk);
	RUN_TEST_GROUP(reset_memory_from_set);
	RUN_TEST_GROUP(create_memory_set_with_malloc_error);
    RUN_TEST_GROUP(create_memory_set_with_malloc_error_and_null_top_context);
	RUN_TEST_GROUP(create_memory_set_with_large_min_size);
	RUN_TEST_GROUP(create_memory_set);
	RUN_TEST_GROUP(allocate_many_small_chunks_with_carving_up);
	RUN_TEST_GROUP(allocate_small_chunk);
	RUN_TEST_GROUP(allocate_many_large_chunks);
	RUN_TEST_GROUP(allocate_large_chunk);
	RUN_TEST_GROUP(create_memory_container);
	RUN_TEST_GROUP(create_cont_with_too_large_size);
	RUN_TEST_GROUP(get_the_most_popular_buffers);
	RUN_TEST_GROUP(get_buffer_from_empty_free_list);
	RUN_TEST_GROUP(get_buffer_from_ring);
	RUN_TEST_GROUP(write_block);
	RUN_TEST_GROUP(find_block_segment);
	RUN_TEST_GROUP(write_file);
	RUN_TEST_GROUP(get_blocks_num_given_the_last_segm_is_filled);
	RUN_TEST_GROUP(get_blocks_num);
	RUN_TEST_GROUP(open_rel_segm);
	RUN_TEST_GROUP(open_rel);
	RUN_TEST_GROUP(file_reopen_closed_file);
	RUN_TEST_GROUP(file_reopen_opened_file);
    RUN_TEST_GROUP(file_open_to_cache_more_than_max);
	RUN_TEST_GROUP(file_open_more_than_allowed);
    RUN_TEST_GROUP(file_close_recent_file);
	RUN_TEST_GROUP(file_estimate_file_count);
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