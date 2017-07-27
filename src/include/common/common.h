
#ifndef COMMON_H
#define COMMON_H

#include <io.h>
#include "icommon.h"

#ifdef _WIN32
#include <windows.h>
#endif

extern const SICommon sCommonHelper;
extern const ICommon  commonHelper;

typedef uintptr_t Datum;
typedef uintptr_t DataPointerIntValue;
typedef char* DataPointer;
typedef DataPointer Item;

#define ConvertIntValueToPointer(intValue) ((DataPointer)(intValue))

#define QuickSort(itemsArray, numberOfItems, elementSize, element) 

#define QuickSortIsLarger 1
#define QuickSortIsSmaller -1
#define QuickSortIsEqual 0

typedef unsigned int ObjectId;
typedef unsigned int TransactionId;

#define InvalidTransactionId ((TransactionId)0)

typedef unsigned char  uint8;
typedef char           int8;
typedef unsigned short uint16;
typedef signed short   int16;
typedef unsigned int   uint;
typedef unsigned long  ulong;
typedef int            Bool;
typedef __int64        int64; 

#define True         1
#define False        0

#define NAME_MAX_LENGTH     64
#define MAX_PATH		    1024
#define ALIGN_DEFAULT_VAL   8
#define ALIGN_INT_VAL       4
#define ALIGN_DOUBLE_VAL    8
#define ALIGN_SHORT_VAL     2
#define ALIGN_BUFFER_VAL	32

#define FlexibleArrayMember

#define DatumGetBool(datum) ((Bool)(GET_1_BYTE(datum) != 0))

#define BoolGetDatum(datum) ((Datum)((datum) ? 1 : 0))

#define CYCLE for(;;) 
#define Max(x, y)		((x) > (y) ? (x) : (y))
#define Min(x, y)		((x) < (y) ? (x) : (y))
#define Abs(x)			((x) >= 0 ? (x) : -(x))

/* 1Gb = 1024Mb = 2^10 Mb = 2^20 Kb = 2^30 bytes
 * We consider that the maximum file size is 1 Gb
 * REL_SEGM_SIZE * BLOCK_SIZE should not be greater than 1 Gb.
 * REL_SEGM_SIZE = 2^30 / 2^13 = 2^(30-13) = 2^17
 */
#define REL_SEGM_SIZE (1 << 17)
#define BlockSize (1 << 13)
#define MAX_BLOCK_SIZE (1 << 15)
#define MAX_PRINTED_CHARS 10
#define MAX_LISTEN 64

#ifdef _WIN32
typedef SOCKET socket_type;
#endif

extern Bool	       IsPostmaster;
extern Bool	       IsSysLogger;
extern int         ProcId;
extern char        ExecPath[MAX_PATH];
extern char*       DataDir;
extern socket_type ListenSockets[MAX_LISTEN];
extern long        CancelKey;
extern int         ChildSlot;
extern int64       StartTime;
extern int64       ReloadTime;
extern int64       LoggerFileTime;
extern Bool		   RedirectDone;
extern int         MaxBackends; 

typedef struct SRelFileInfo
{
	int		      tblSpaceId;
	int 	      databaseId;
	int		      relId;		
} SRelFileInfo, *RelFileInfo;

typedef struct SRelFileInfoBack
{
	SRelFileInfo  node;
	int	          backend;
} SRelFileInfoBack, *RelFileInfoBack;

typedef struct SFileSeg
{
	/* File's index in the file cache.
	 * Not file descriptor.
	 */
	int		          find;
	uint              num;
	char*             fname;
	struct SFileSeg*  next;
} SFileSeg, *FileSeg;

typedef FileSeg *AFileSeg;

/* This is the same char* but the length restrictions is provided */
typedef struct SName
{
	char	    value[NAME_MAX_LENGTH];
} SName, *Name;

#define VarSizeAny(dataPointer) (0)
#define VarSize(dataPointer) (0)

#define ALIGN(VAL,LEN)  \
	(((int)(LEN) + ((VAL) - 1)) & ~((int)((VAL) - 1)))

/* Aligns a number to the lower value. 
 * For example:
 *  VAL       = 8. ~(VAL - 1) = ~(00..00111) = 111...111000
 *  LEN & VAL clears a reminder of dividing on 8.
 *  LEN = 20 = 10100 & 11000 = 10000 = 16
 */
#define ALIGN_DOWN(VAL,LEN)  \
	(((int)(LEN)) & ~((int)((VAL) - 1)))

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
#define AlignDefault(len)       ALIGN(ALIGN_DEFAULT_VAL, (len))
#define ALIGN_INT(LEN)          ALIGN(ALIGN_INT_VAL, (LEN)) 
#define ALIGN_DOUBLE(LEN)       ALIGN(ALIGN_DOUBLE_VAL, (LEN)) 
#define ALIGN_SHORT(LEN)        ALIGN(ALIGN_SHORT_VAL, (LEN)) 
#define ALIGN_BUFFER(LEN)		ALIGN(ALIGN_BUFFER_VAL, (LEN))

#define AlignDownDefault(length) ALIGN_DOWN(ALIGN_DEFAULT_VAL, (length))

#define GET_1_BYTE(val) (((uint)(val)) & 0x000000ff)
#define GET_2_BYTES(val) (((uint)(val)) & 0x0000ffff)
#define GET_4_BYTES(val) (((uint)(val)) & 0xffffffff)

#define SET_1_BYTE(val) (((uint)(val)) & 0x000000ff)
#define SET_2_BYTES(val) (((uint)(val)) & 0x0000ffff)
#define SET_4_BYTES(val) (((uint)(val)) & 0xffffffff)


#define FILE_EXISTS(name) ((access((name), _A_NORMAL) != -1) ? True : False)

#define Int32GetDatum(data) ((Datum)SET_4_BYTES(data))

#define Int16GetDatum(data) ((Datum)SET_2_BYTES(data))

#define CharGetDatum(data) ((Datum)SET_1_BYTE(data))

#define PointerGetDatum(data) ((Datum)(data))

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
void setExecFold(char* fold);
char* getExecFold();
void fillCommonParams(int argc, char* argv[]);

/* Generate assempler code by writing ANCI C program
 * gcc -Wall -S -masm=intel -m32 sub.c
 */

#endif
