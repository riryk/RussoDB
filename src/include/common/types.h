
#ifndef TYPES_H
#define TYPES_H

#include "common.h"

typedef struct SType_1b
{
	uint8		header;
	char		data[1];		
} SType_1b, *Type_1b;

typedef struct SType_2b
{
	uint8		header;		
	uint8		byte2;		
	char		data[1];		
} SType_2b, *Type_2b;

typedef union SType_4b
{
	struct					
	{
		uint		header;
		char		data[1];
	} value;
	struct						
	{
		uint		header;
		uint		size; 
		char		data[1]; 
	} compressed;
} SType_4b, *Type_4b;

/* Let's take some number: 0001 1100 0100.
 * If we convert it to Type_1b we will receive:
 *    header = 1100 0100, 
 *    data   = 00...0001
 *    0x03 = 00...0011 
 *    header & 0x03 =  1100 0100 = 0 
 *                     0000 0011 
 * If at least one of the first 2 bits is 1, the macros will
 * return false. For example:
 *    1100 0101 = 0000 0001
 *    0000 0011
 */
#define ARE_FIRST_2_BITS_ZEROS(p) \
    ((((Type_1b)(p))->header & 0x03) == 0x00)

/* This macros checks if the first bit is 1. */
#define IS_FIRST_BIT_1(p) \
	((((Type_1b)(p))->header & 0x01) == 0x01)

#define IS_FIRST_BIT_0(p) \
	((((Type_1b)(p))->header & 0x01) == 0x00)

/* If we have a number and the first 8 bits are: 0000 0001,
 * then this macro will return 0, otherwise false. 
 */
#define IS_FIRST_BYTE_1(p) \
	((((Type_1b)(p))->header) == 0x01)

#define ONE 1

/* In Binary system it looks like:  
 * 0111 1111
 */
#define ONLY_FIRST_7_BITS_PATTERN 0x7F

/* For example we have an arbitrary number:
 * 5717 = 0001 0110 0101 0101
 * 0101 010 = 42 number will be returned.
 */
#define CUT_THE_LAST_BIT_AND_TAKE_7_BITS(p) \
	((((Type_1b)(p))->header >> ONE) & ONLY_FIRST_7_BITS_PATTERN)

/* Retrieves the second byte from a number.
 * For example if we have 16 bits number: 1011 1100 0011 0001
 * Bits from position 8 to position 16 will be returned.
 */
#define GET_SECOND_BYTE(p) \
   	(((Type_2b)(p))->byte2)

#define TWO 2

/* In Binary system it looks like:  
 * 0011 1111 1111 1111 1111 1111 1111 1111
 */
#define ONLY_FIRST_30_BITS_PATTERN 0x3FFFFFFF
/*    If we have a 4 byte integer value: 
 *      Bin        = 1101 1100 1011 1001 1101 1100 1011 1001
 *      Bin >> 2   = 0011 0111 0010 1110 0111 0111 0010 1110   
 *      0x3FFFFFFF = 0011 1111 1111 1111 1111 1111 1111 1111
 *    Bitwise anding with 0x3FFFFFFF is needed when we have 8 byte number.
 *    We shorten it to 30 bits provided that the last 2 bits are zeros.
 */
#define CUT_LAST_2_BITS_AND_TAKE_30_NEXT_BITS(p) \
	((((Type_4b)(p))->value.header >> TWO) & ONLY_FIRST_30_BITS_PATTERN)

/* On most computers sizeof(int) = 4. sizeof(int) - 1 = 3.
 * We cut the first 2 bits and convert the number to 30 bit number.
 * After we substract 3.
 */
#define SHORT_SIZE(p) \
	(CUT_LAST_2_BITS_AND_TAKE_30_NEXT_BITS(p) - ((int)sizeof(int) - 1))

/* short integer occupies 2 bytes, 16 bits.
 * For signed integer we have: -127 <= x <= 127 
 * We consider 127 as the maximum allowed number:
 * In Binary system it looks like: 0111 1111 */
#define MAX_SHORT 0x7F

/* We can convert integer value to short 
 * when the first 2 bits are zeros and the short size
 * of this number is less or equal 127.
 */
#define CAN_COMPRESS_TO_SHORT(p) \
	(ARE_FIRST_2_BITS_ZEROS(p) && SHORT_SIZE(p) <= MAX_SHORT)

/* If the first byte is 1 we take the second byte.
 * else if the first bit is 1 we take bits from 8 to 1
 * else we take bits from 32 to 2.
 */
#define SIZE(p) \
	(IS_FIRST_BYTE_1(p) ? GET_SECOND_BYTE(p) \
	                    : (IS_FIRST_BIT_1(p) ? CUT_THE_LAST_BIT_AND_TAKE_7_BITS(p) \
						                     : CUT_LAST_2_BITS_AND_TAKE_30_NEXT_BITS(p)))

#define SET_VARSIZE_1B(p,len) \
	(((Type_1b)p)->header = (((uint8)len) << 1) | 0x01)

#define VARDATA_4B(p) \
	(((Type_4b)p)->value.data)


#endif
