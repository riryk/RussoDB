#include "hashtable.h"
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "relrowmanager.h"
#include "relrow_helper.h"

RelAttribute attrs;
size_t       size;

uint         csflvsa_a1 = 536;
uint         csflvsa_a2 = 592;
uint         csflvsa_a3 = 536;
uint         csflvsa_a4 = 600;
uint         csflvsa_a5 = 624;
uint         csflvsa_a6 = 696;

uint         csflvsa_values[6] = 
{ 
	&csflvsa_a1,      // 0000 0010 0001 1000  536 / 4 = 134
	&csflvsa_a2,      // 0000 0010 0101 0000  592 / 4 = 148
	&csflvsa_a3,      // 0000 0010 0001 1000  536 / 4 = 134
	&csflvsa_a4,      // 0000 0010 0101 1000  600 / 4 = 150 
	&csflvsa_a5,      // 0000 0010 0111 0000  624 / 4 = 156
	&csflvsa_a6       // 0000 0010 1011 1000  696 / 4 = 174
};
           
TEST_GROUP(compute_size_for_long_var_size_attr);

SETUP_DEPENDENCIES(compute_size_for_long_var_size_attr) 
{
}

GIVEN(compute_size_for_long_var_size_attr)
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

WHEN(compute_size_for_long_var_size_attr) 
{
	size = computeRowSize(attrs, 6, csflvsa_values, NULL);
}

TEST_TEAR_DOWN(compute_size_for_long_var_size_attr)
{
	sFakeMemManager.freeAll();
}

TEST(compute_size_for_long_var_size_attr, 
	 then_len_must_be_extracted_from_the_next_30_bits)
{
    int expectedSize = 134  +
                       148  +
                       134  +
                       150  +
                       156  +
                       174; 

	TEST_ASSERT_EQUAL_UINT32(size, expectedSize);
}

TEST_GROUP_RUNNER(compute_size_for_long_var_size_attr)
{
    RUN_TEST_CASE(compute_size_for_long_var_size_attr, 
		          then_len_must_be_extracted_from_the_next_30_bits);
}


