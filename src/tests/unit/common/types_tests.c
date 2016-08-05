#include "unity_fixture.h"
#include "types.h"
#include "relrow.h"

TEST_GROUP(types_tests);

SETUP_DEPENDENCIES(types_tests) { }
GIVEN(types_tests) { }
WHEN(types_tests) { }
TEST_TEAR_DOWN(types_tests) { }

typedef struct STest
{ 
    uint             len;
    uint		 	 field1;
    uint		 	 field2;
    uint		 	 field3;
} STest, *Test;

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
    int       ssmall = 52;          //  0011 0100
	long long big    = 42357873851; //  1001 1101 1100 1011 1001 1101 1100 1011 1011

    int smallAfter = CUT_LAST_2_BITS_AND_TAKE_30_NEXT_BITS(&ssmall);
    int bigAfter   = CUT_LAST_2_BITS_AND_TAKE_30_NEXT_BITS(&big);

    TEST_ASSERT_EQUAL_UINT32(smallAfter, 13);      //1101
	TEST_ASSERT_EQUAL_UINT32(bigAfter, 925792046); // 0011 0111 0010 1110 0111 0111 0010 1110
}

TEST(types_tests, shortsize)
{
    int       ssmall = 52;          //  0011 0100
	long long big    = 42357873851; //  1001 1101 1100 1011 1001 1101 1100 1011 1011

    int smallAfter = LONG_VAR_LEN_ATTR_SIZE(&ssmall);
    int bigAfter   = LONG_VAR_LEN_ATTR_SIZE(&big);

    TEST_ASSERT_EQUAL_UINT32(smallAfter, 13 - 3);      //1101 - 11
	TEST_ASSERT_EQUAL_UINT32(bigAfter, 925792046 - 3); // 0011 0111 0010 1110 0111 0111 0010 1110 - 11
}

TEST(types_tests, can_compress_to_short)
{
    int       ssmall        = 52;
	int       big           = 1445;

	Bool      smallAsShort = CAN_COMPRESS_TO_SHORT(&ssmall);
    Bool      bigAsShort   = CAN_COMPRESS_TO_SHORT(&big);

	TEST_ASSERT_TRUE(smallAsShort);
    TEST_ASSERT_FALSE(bigAsShort);
} 

TEST(types_tests, is_first_bit_1)
{
    int       small1   = 52;          //  0011 0100
    int       small2   = 53;          //  0011 0101

	long long big1     = 42357873851; //  1001 1101 1100 1011 1001 1101 1100 1011 1011
    long long big2     = 42357873848; //  1001 1101 1100 1011 1001 1101 1100 1011 1000

	Bool      isSmall1 = IS_FIRST_BIT_1(&small1); 
    Bool      isSmall2 = IS_FIRST_BIT_1(&small2); 

    Bool      isBig1   = IS_FIRST_BIT_1(&big1); 
    Bool      isBig2   = IS_FIRST_BIT_1(&big2); 

    TEST_ASSERT_FALSE(isSmall1);   
    TEST_ASSERT_TRUE(isSmall2);   

    TEST_ASSERT_FALSE(isBig2);   
    TEST_ASSERT_TRUE(isBig1);   
}

TEST(types_tests, is_first_byte_1)
{
    int       small1   = 1;          //  0000 0001
	int       small2   = 163;        //  1010 0011

	int       big1     = 573046785;  // 0010 0010 0010 1000 0000 0000 0000 0001
    int       big2     = 573047305;  // 0010 0010 0010 1000 0000 0010 0000 1001

    Bool      isSmall1 = IS_FIRST_BYTE_1(&small1); 
    Bool      isSmall2 = IS_FIRST_BYTE_1(&small2); 

    Bool      isBig1   = IS_FIRST_BYTE_1(&big1); 
    Bool      isBig2   = IS_FIRST_BYTE_1(&big2); 

	TEST_ASSERT_TRUE(isSmall1);   
    TEST_ASSERT_FALSE(isSmall2);   

    TEST_ASSERT_TRUE(isBig1);   
    TEST_ASSERT_FALSE(isBig2);  
}

TEST(types_tests, get_second_bype)
{
    int       ssmall      = 16725;     //            0100 0001 0101 0101
	int       large       = 4474205;   //  0100 0100 0100 0101 0101 1101

    int       smallByte2 = GET_SECOND_BYTE(&ssmall); // 0100 0001
    int       largeByte2 = GET_SECOND_BYTE(&large); // 0100 0101

    TEST_ASSERT_EQUAL_UINT32(smallByte2, 65); 
    TEST_ASSERT_EQUAL_UINT32(largeByte2, 69); 
}

TEST(types_tests, cut_the_last_bit_and_take_7_bits)
{
    int       ssmall      = 16725;     //            0100 0001 0101 0101
	int       large       = 4474205;   //  0100 0100 0100 0101 0101 1101

    int       smallByte2 = CUT_THE_LAST_BIT_AND_TAKE_7_BITS(&ssmall); // 0010 1010
    int       largeByte2 = CUT_THE_LAST_BIT_AND_TAKE_7_BITS(&large);  // 0010 1110
    
    TEST_ASSERT_EQUAL_UINT32(smallByte2, 42); 
    TEST_ASSERT_EQUAL_UINT32(largeByte2, 46);    
}

TEST(types_tests, size)
{
	int       firstByte1          = 55809;     //                     1101 1010 0000 0001    
	int       firstBit1           = 16725;     //                     0100 0001 0101 0101   
    int       large               = 573047306; // 0010 0010 0010 1000 0000 0010 0000 1010

	int       firstByte1Size      = ComputeSize(&firstByte1);
    int       firstBit1Size       = ComputeSize(&firstBit1);
    int       largeSize           = ComputeSize(&large); 

	int       firstByte1Expected  = 218;       // 1101 1010
    int       firstBit1Expected   = 42;        // 0010 1010
    int       largeExpected       = 143261826; // 0000 1000 1000 1010 0000 0000 1000 0010

	TEST_ASSERT_EQUAL_UINT32(firstByte1Size, firstByte1Expected); 
    TEST_ASSERT_EQUAL_UINT32(firstBit1Size,  firstBit1Expected); 
    TEST_ASSERT_EQUAL_UINT32(largeSize,      largeExpected); 
}

TEST(types_tests, set_first_4_bytes)
{
    STest         ttSmall, ttLarge;
	SRelRowHeader relRowHd;

    uint    lenSmall      = 124;        // 0000 0111 1100
    uint    lenLarge      = 3794272778; // 1110 0010 0010 1000 0000 0010 0000 1010
    uint    expectedSmall = 496;        // 0001 1111 0000
	uint    expectedLarge = 2292189224; // 1000 1000 1010 0000 0000 1000 0010 1000

	memset(&ttSmall, 0, sizeof(STest));
    memset(&ttLarge, 0, sizeof(STest));
    memset(&relRowHd, 0, sizeof(SRelRowHeader));

    SET_FIRST_4_BYTES(&ttSmall, lenSmall);
    SET_FIRST_4_BYTES(&ttLarge, lenLarge);
    SET_FIRST_4_BYTES(&relRowHd, lenSmall);

	TEST_ASSERT_EQUAL_UINT32(ttSmall.len, expectedSmall); 
	TEST_ASSERT_EQUAL_UINT32(ttLarge.len, expectedLarge); 
	TEST_ASSERT_EQUAL_UINT32(relRowHd.typeData.data.len, expectedSmall); 
}

TEST_GROUP_RUNNER(types_tests)
{
    RUN_TEST_CASE(types_tests, are_first_2_bits_zeros);
    RUN_TEST_CASE(types_tests, cut_last_2_bits_and_take_30_next_bits);
    RUN_TEST_CASE(types_tests, shortsize);
    RUN_TEST_CASE(types_tests, can_compress_to_short);
	RUN_TEST_CASE(types_tests, is_first_bit_1);
    RUN_TEST_CASE(types_tests, is_first_byte_1);
    RUN_TEST_CASE(types_tests, get_second_bype);
	RUN_TEST_CASE(types_tests, cut_the_last_bit_and_take_7_bits);
    RUN_TEST_CASE(types_tests, size);
    RUN_TEST_CASE(types_tests, set_first_4_bytes);
}





