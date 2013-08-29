
#include <stdlib.h>
#include <string.h>

#ifndef COMMON_H
#define COMMON_H

typedef unsigned int  uint;
typedef unsigned long ulong;
typedef int           Bool;

#define True         1
#define False        0

#define NAME_MAX_LENGTH  64
#define ALIGN_VAL        8

#define Max(x, y)		((x) > (y) ? (x) : (y))
#define Min(x, y)		((x) < (y) ? (x) : (y))
#define Abs(x)			((x) >= 0 ? (x) : -(x))



/* This is the same char* but the length restrictions is provided */
typedef struct SName
{
	char	   value[NAME_MAX_LENGTH];
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


#define catalog_relation \
{ 1000, {"name"},      19, -1, False, False, False, 0 }, \
{ 1000, {"namespace"}, 26, -1, False, False, False, 0 }, \
{ 1000, {"type"},      26, -1, False, False, False, 0 }

int nextPowerOf2(long num);

#endif