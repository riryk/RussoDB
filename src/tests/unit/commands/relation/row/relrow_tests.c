#include "unity_fixture.h"
#include "relrow.h"

TEST_GROUP(relrow_tests);

SETUP_DEPENDENCIES(relrow_tests) { }
GIVEN(relrow_tests) { }
WHEN(relrow_tests) { }
TEST_TEAR_DOWN(relrow_tests) { }

TEST(relrow_tests, att_align)
{
    int align_i1 = ATT_ALIGN(11, 'i');
    int align_i2 = ATT_ALIGN(9, 'i');

	int align_d1 = ATT_ALIGN(22, 'd');
    int align_d2 = ATT_ALIGN(17, 'd');

	int align_s1 = ATT_ALIGN(12, 's');
    int align_s2 = ATT_ALIGN(11, 's');
    int align_s3 = ATT_ALIGN(9, 's');

    int align_o = ATT_ALIGN(12, 'o');

    TEST_ASSERT_EQUAL_UINT32(align_i1, 12);  
    TEST_ASSERT_EQUAL_UINT32(align_i2, 12);  

    TEST_ASSERT_EQUAL_UINT32(align_d1, 24);  
    TEST_ASSERT_EQUAL_UINT32(align_d2, 24);  

    TEST_ASSERT_EQUAL_UINT32(align_s1, 12);  
    TEST_ASSERT_EQUAL_UINT32(align_s2, 12); 
	TEST_ASSERT_EQUAL_UINT32(align_s3, 10);  

    TEST_ASSERT_EQUAL_UINT32(align_o, 12);  
}

TEST_GROUP_RUNNER(relrow_tests)
{
    RUN_TEST_CASE(relrow_tests, att_align);
}





