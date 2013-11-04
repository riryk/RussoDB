#include "hashtable.h"
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "relrowmanager.h"
#include "relrow_helper.h"

RelAttribute attrs;
size_t       size;

uint         csfsvsa_a1 = 24;
uint         csfsvsa_a2 = 80;
uint         csfsvsa_a3 = 24;
uint         csfsvsa_a4 = 88;
uint         csfsvsa_a5 = 112;
uint         csfsvsa_a6 = 184;

uint         csfsvsa_values[6] = 
{ 
	&csfsvsa_a1,      // 0001 1000
	&csfsvsa_a2,      // 0101 0000
	&csfsvsa_a3,      // 0001 1000
	&csfsvsa_a4,      // 0101 1000
	&csfsvsa_a5,      // 0111 0000
	&csfsvsa_a6       // 1011 1000
};

TEST_GROUP(compute_size_for_shortable_var_size_attr);

SETUP_DEPENDENCIES(compute_size_for_shortable_var_size_attr) 
{
}

GIVEN(compute_size_for_shortable_var_size_attr)
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

WHEN(compute_size_for_shortable_var_size_attr) 
{
	size = computeRowSize(attrs, 6, csfsvsa_values, NULL);
}

TEST_TEAR_DOWN(compute_size_for_shortable_var_size_attr)
{
	sFakeMemManager.freeAll();
}

TEST(compute_size_for_shortable_var_size_attr, 
	 then_len_must_be_extracted_from_the_next_7_bits)
{
    int expectedSize = 3   + // 0000 0110   6  - 3 = 3
                       17  + // 0001 0100   20 - 3 = 17
                       3   + // 0000 0110   6  - 3 = 3 
                       19  + // 0001 0110   22 - 3 = 19
                       25  + // 0001 1100   28 - 3 = 25
                       43;   // 0010 1110   46 - 3 = 43

	TEST_ASSERT_EQUAL_UINT32(size, expectedSize);
}

TEST_GROUP_RUNNER(compute_size_for_shortable_var_size_attr)
{
    RUN_TEST_CASE(compute_size_for_shortable_var_size_attr, 
		          then_len_must_be_extracted_from_the_next_7_bits);
}


