
#include <stdlib.h>
#include <string.h>
#include "icommon.h"

#ifndef COMMON_H
#define COMMON_H

extern const SICommon sCommonHelper;
extern const ICommon  commonHelper;

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef signed short   int16;
typedef unsigned int   uint;
typedef unsigned long  ulong;
typedef int            Bool;

#define True         1
#define False        0

#define NAME_MAX_LENGTH  64
#define ALIGN_VAL        8

#define Max(x, y)		((x) > (y) ? (x) : (y))
#define Min(x, y)		((x) < (y) ? (x) : (y))
#define Abs(x)			((x) >= 0 ? (x) : -(x))


typedef struct SCol_1b
{
	uint8		header;
	char		data;		/* Data begins here */
} SCol_1b, *Col_1b;


typedef union SCol_4b
{
	struct					
	{
		uint32		header;
		char		data;
	} col_4byte;
	struct						
	{
		uint32		header;
		uint32		size; 
		char		data; 
	} compressed;
} SCol_4b, *Col_4b;

#define IS_4B(p) \
    ((((Col_1b)(p))->header & 0x03) == 0x00)

#define VARSIZE_4B(p) \
	((((Col_4b)(p))->col_4byte.header >> 2) & 0x3FFFFFFF)

#define CAN_MAKE_SHORT(p) \
	(IS_4B(p) && \
	 (VARSIZE_4B(p) - (int)sizeof(int) + 1) <= 0x7F)

#define SHORT_SIZE(PTR) \
	(VARSIZE(PTR) - VARHDRSZ + VARHDRSZ_SHORT)

typedef struct SBlockId
{
	uint16		 high;
	uint16		 low;
} SBlockId, *BlockId;

typedef struct SRowPointer
{
	SBlockId     block;
	uint16       pos;
} SRowPointer, *RowPointer

/* This is the same char* but the length restrictions is provided */
typedef struct SName
{
	char	    value[NAME_MAX_LENGTH];
} SName, *Name;


/* This macro is used to facilitate calculation 
 * and more easily reuse some memory.
 * For example: if ALIGN_VAL is 8. We want all memory 
 * pieces to be 8n where n is an integer number.
 * How it works. Let's take an arbitrary number 
 * LEN = 00...0bnbn-1bn-2...b3b2b1b0, bi is 0 or 1
 * In binary system it has length of n. 
 * ALIGN_VAL = 8 = 1000, 
 * 8 - 1 = 0000...00111
 * ~7    = 1111...11000
 * We can write LEN as 8n + k, where 0 <= k <= 7
 * and we add 7 to it where 0 <= m <= 7. 
 * A new number is: 8n + k + 7. When k == 0 then 
 * 8n + 7 and by bit anding ~7 we clear the last 3 bits and get 8n
 * When we have k > 0 then a new number is: 8n + (k - 1) + 8 = 
 * = 8(n + 1) + (k - 1), 0 <= k - 1 <= 8. We clear k - 1 and 
 * receive 8(n + 1) 
 */
#define ALIGN(LEN)  \
	(((int)(LEN) + ((ALIGN_VAL) - 1)) & ~((int)((ALIGN_VAL) - 1)))

#define cat_rel_attr_count   8

#define catalog_relation \
{ 1000, {"name"},      19, -1, False, False, False, 0 }, \
{ 1000, {"namespace"}, 26, -1, False, False, False, 0 }, \
{ 1000, {"type"},      26, -1, False, False, False, 0 }


#define catalog_column \
{ 1001, {"relid"},      26, -1, False, False, False, 0 }, \
{ 1001, {"name"},       19, -1, False, False, False, 0 }, \
{ 1001, {"type"},       26, -1, False, False, False, 0 }, \
{ 1001, {"stattarget"}, 23, -1, False, False, False, 0 }, \
{ 1001, {"length"},     21, -1, False, False, False, 0 }, \
{ 1001, {"mode"},       23, -1, False, False, False, 0 }, \
{ 1001, {"align"},      18, -1, False, False, False, 0 }, \
{ 1001, {"notnull"},    16, -1, False, False, False, 0 }, \
{ 1001, {"collation"},  26, -1, False, False, False, 0 }


#define catalog_proc \
{ 1002, {"name"},        19,   -1, False, False, False, 0 }, \
{ 1002, {"namespace"},   26,   -1, False, False, False, 0 }, \
{ 1002, {"owner"},       26,   -1, False, False, False, 0 }, \
{ 1002, {"language"},    26,   -1, False, False, False, 0 }, \
{ 1002, {"volatile"},    18,   -1, False, False, False, 0 }, \
{ 1002, {"nargs"},       21,   -1, False, False, False, 0 }, \
{ 1002, {"defaults"},    21,   -1, False, False, False, 0 }, \
{ 1002, {"rettype"},     26,   -1, False, False, False, 0 }, \
{ 1002, {"argtypes"},    30,   -1, False, False, False, 0 }, \
{ 1002, {"argmodes"},    1002, -1, False, False, False, 0 }, \
{ 1002, {"argnames"},    1009, -1, False, False, False, 0 }, \
{ 1002, {"argdefaults"}, 194,  -1, False, False, False, 0 }, \
{ 1002, {"src"},         25,   -1, False, False, False, 0 }, \
{ 1002, {"bin"},         25,   -1, False, False, False, 0 }, \
{ 1002, {"config"},      1009, -1, False, False, False, 0 }


#define catalog_type \
{ 1003, {"name"},        19,  -1, False, False, False, 0 }, \
{ 1003, {"namespace"},   26,  -1, False, False, False, 0 }, \
{ 1003, {"owner"},       26,  -1, False, False, False, 0 }, \
{ 1003, {"length"},      21,  -1, False, False, False, 0 }, \
{ 1003, {"type"},        18,  -1, False, False, False, 0 }, \
{ 1003, {"category"},    18,  -1, False, False, False, 0 }, \
{ 1003, {"isdefined"},   16,  -1, False, False, False, 0 }, \
{ 1003, {"notnull"},     16,  -1, False, False, False, 0 }, \
{ 1003, {"basetype"},    26,  -1, False, False, False, 0 }, \
{ 1003, {"mode"},        23,  -1, False, False, False, 0 }, \
{ 1003, {"collation"},   26,  -1, False, False, False, 0 }, \
{ 1003, {"defaultbin"},  194, -1, False, False, False, 0 }, \
{ 1003, {"default"},     25,  -1, False, False, False, 0 }


int nextPowerOf2(long num);

/* Generate assempler code by writing ANCI C program
 * gcc -Wall -S -masm=intel -m32 sub.c
 */

#endif
