#include "unity_fixture.h"
#include "relrow.h"

TEST_GROUP(relrow_tests);

SETUP_DEPENDENCIES(relrow_tests) { }
GIVEN(relrow_tests) { }
WHEN(relrow_tests) { }
TEST_TEAR_DOWN(relrow_tests) { }

TEST(relrow_tests, att_align)
{
    int align_i = ATT_ALIGN(12, 'i');
	int align_d = ATT_ALIGN(22, 'd');
	int align_s = ATT_ALIGN(11, 's');
    int align_o = ATT_ALIGN(12, 'o');

    TEST_ASSERT_EQUAL_UINT32(align_i, 16);  
    TEST_ASSERT_EQUAL_UINT32(align_d, 32);  
    TEST_ASSERT_EQUAL_UINT32(align_s, 12);  
    TEST_ASSERT_EQUAL_UINT32(align_o, 12);  
}

TEST_GROUP_RUNNER(relrow_tests)
{
    RUN_TEST_CASE(relrow_tests, att_align);
}





