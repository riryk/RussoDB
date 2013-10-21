#include "unity_fixture.h"
#include "types.h"

TEST_GROUP(types_tests);

SETUP_DEPENDENCIES(types_tests) { }
GIVEN(types_tests) { }
WHEN(types_tests) { }
TEST_TEAR_DOWN(types_tests) { }

TEST(types_tests, are_first_2_bits_zeros)
{
	int last2bitsnotzero_1byte = 146;
	int last2bitszero_1byte    = 220;

	int last2bitsnotzero_4byte = 18752557645;
    int last2bitszero_4byte    = 18752557644;

	Type_1b type1 = (Type_1b)(char*)&last2bitsnotzero_1byte;
	Type_1b type2 = (Type_1b)(char*)&last2bitszero_1byte;

    Type_1b type3 = (Type_1b)(char*)&last2bitsnotzero_4byte;
	Type_1b type4 = (Type_1b)(char*)&last2bitszero_4byte;

	Bool areZeros1 = ARE_FIRST_2_BITS_ZEROS((char*)&last2bitsnotzero_1byte);
    Bool areZeros2 = ARE_FIRST_2_BITS_ZEROS((char*)&last2bitszero_1byte);

    Bool areZeros3 = ARE_FIRST_2_BITS_ZEROS((char*)&last2bitsnotzero_4byte);
    Bool areZeros4 = ARE_FIRST_2_BITS_ZEROS((char*)&last2bitszero_4byte);

    TEST_ASSERT_FALSE(areZeros1);
    TEST_ASSERT_TRUE(areZeros2);

    TEST_ASSERT_FALSE(areZeros3);
    TEST_ASSERT_TRUE(areZeros4);
}

TEST(types_tests, cut_last_2_bits_and_take_30_next_bits)
{
    int small = 52; // 110100

}

TEST_GROUP_RUNNER(types_tests)
{
    RUN_TEST_CASE(types_tests, are_first_2_bits_zeros);
    RUN_TEST_CASE(types_tests, cut_last_2_bits_and_take_30_next_bits);
}





