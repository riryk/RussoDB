#include "hashtable.h"
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "relrowmanager.h"
#include "relrow_helper.h"

RelAttribute attrs;
size_t       size;

uint         csfvsawtfbi1_a1 = 13;
uint         csfvsawtfbi1_a2 = 81;
uint         csfvsawtfbi1_a3 = 25;
uint         csfvsawtfbi1_a4 = 89;
uint         csfvsawtfbi1_a5 = 113;
uint         csfvsawtfbi1_a6 = 189;

uint         csfvsawtfbi1_values[6] = 
{ 
	&csfvsawtfbi1_a1,      // 0000 1101
	&csfvsawtfbi1_a2,      // 0101 0001
	&csfvsawtfbi1_a3,      // 0001 1001
	&csfvsawtfbi1_a4,      // 0101 1001
	&csfvsawtfbi1_a5,      // 0111 0001
	&csfvsawtfbi1_a6       // 1011 1101
};

TEST_GROUP(compute_size_for_var_size_attr_with_the_first_bit_1);

SETUP_DEPENDENCIES(compute_size_for_var_size_attr_with_the_first_bit_1) 
{
}

GIVEN(compute_size_for_var_size_attr_with_the_first_bit_1)
{
	attrs = (RelAttribute)sFakeMemManager.alloc(6 * sizeof(SRelAttribute));
	memset(attrs, 0, 6 * sizeof(SRelAttribute));

    attrs[0].len = -1;
	attrs[1].len = -1;
	attrs[2].len = -1;
	attrs[3].len = -1;
	attrs[4].len = -1;
    attrs[5].len = -1;
}

WHEN(compute_size_for_var_size_attr_with_the_first_bit_1) 
{
	size = computeRowSize(attrs, 6, csfvsawtfbi1_values, NULL);
}

TEST_TEAR_DOWN(compute_size_for_var_size_attr_with_the_first_bit_1)
{
	sFakeMemManager.freeAll();
}

TEST(compute_size_for_var_size_attr_with_the_first_bit_1, 
	 then_len_must_be_extracted_from_the_next_7_bits)
{
    int expectedSize = 6   + // 0000 0110   
                       40  + // 0010 1000
                       12  + // 0000 1100   
                       44  + // 0010 1100
                       56  + // 0011 1000
                       94;   // 0101 1110

	TEST_ASSERT_EQUAL_UINT32(size, expectedSize);
}

TEST_GROUP_RUNNER(compute_size_for_var_size_attr_with_the_first_bit_1)
{
    RUN_TEST_CASE(compute_size_for_var_size_attr_with_the_first_bit_1, 
		          then_len_must_be_extracted_from_the_next_7_bits);
}


