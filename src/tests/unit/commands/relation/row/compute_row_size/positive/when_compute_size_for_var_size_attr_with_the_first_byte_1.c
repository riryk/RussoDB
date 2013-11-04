#include "hashtable.h"
#include "unity_fixture.h"
#include "fakememmanager.h"
#include "relrowmanager.h"
#include "relrow_helper.h"

RelAttribute attrs;
size_t       size;

uint         a1 = 1025;
uint         a2 = 5121;
uint         a3 = 13313;
uint         a4 = 44033;
uint         a5 = 19457;
uint         a6 = 38913;

uint         csfvsawtfb1_values[6] = 
{ 
	&a1,      // 0000 0100 0000 0001
	&a2,      // 0001 0100 0000 0001
	&a3,      // 0011 0100 0000 0001
	&a4,      // 1010 1100 0000 0001
	&a5,      // 0100 1100 0000 0001
	&a6       // 1001 1000 0000 0001
};

TEST_GROUP(compute_size_for_var_size_attr_with_the_first_byte_1);

SETUP_DEPENDENCIES(compute_size_for_var_size_attr_with_the_first_byte_1) 
{
}

GIVEN(compute_size_for_var_size_attr_with_the_first_byte_1)
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

WHEN(compute_size_for_var_size_attr_with_the_first_byte_1) 
{
	size = computeRowSize(attrs, 6, csfvsawtfb1_values, NULL);
}

TEST_TEAR_DOWN(compute_size_for_var_size_attr_with_the_first_byte_1)
{
	sFakeMemManager.freeAll();
}

TEST(compute_size_for_var_size_attr_with_the_first_byte_1, 
	 then_len_must_be_extracted_from_the_second_byte)
{
    int expectedSize = 4   + // 0000 0100   
                       20  + // 0001 0100
                       52  + // 0011 0100   
                       172 + // 1010 1100
                       76  + // 0100 1100
                       152;  // 1001 1000

	TEST_ASSERT_EQUAL_UINT32(size, expectedSize);
}

TEST_GROUP_RUNNER(compute_size_for_var_size_attr_with_the_first_byte_1)
{
    RUN_TEST_CASE(compute_size_for_var_size_attr_with_the_first_byte_1, 
		          then_len_must_be_extracted_from_the_second_byte);
}


