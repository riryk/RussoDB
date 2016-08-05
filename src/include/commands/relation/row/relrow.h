
#ifndef IRELROW_H
#define IRELROW_H

#include "common.h"
#include "block.h"

#define ROW_ATTRS_MAX_COUNT 1600
/* 0x80 = 1000 0000 */
#define BIT_8_SET 0x80

/* 0x07FF in binary system looks like:  0111 1111 1111 
 * Row header has a mask attribute uint16.
 * The first 11 bits are reserved for a relation attributes count.
 * So that the largest attributes count is 2047.
 */
#define REL_ATTR_COUNT_MASK    0x07FF	

/* row bit flags */
#define ROW_HASNULL			   0x0001   /* 0000 0000 0000 0001 */
#define ROW_HASVARWIDTH		   0x0002	/* 0000 0000 0000 0010 */
#define ROW_HASEXTERNAL		   0x0004	/* 0000 0000 0000 0100 */
#define ROW_HASID			   0x0008	/* 0000 0000 0000 1000 */
#define ROW_TRAN_MIN_INVALID   0x0200	/* 0000 0010 0000 0000 */

/* Bits from 6 to 16 are reserved for transaction information.
 * num & ~(ROW_TRAN_MASK) clears those bits. 
 * num |= ROW_TRAN_MIN_INVALID sets of these bits. 
 */
#define ROW_TRAN_MASK		   0xFFE0	/* 1111 1111 1110 0000 */
#define ROW_TRAN_MASK2		   0xC000	/* 1100 0000 0000 0000 */

/* Retrieves the first 11 bits from rel row header mask2
 * They represent a relation attributes count.
 */
#define GET_REL_ATTR_COUNT(rHd) \
    ((rHd)->mask2 & REL_ATTR_COUNT_MASK) 

/* This macros sets the first 11 bits of a row header.
 * From start we have mask2 = 0101 1001 0100 0010
 * The first 11 bits are the old value for a relation attributes count.
 *  REL_ATTR_COUNT_MASK = 0000 0111 1111 1111
 * ~REL_ATTR_COUNT_MASK = 1111 1000 0000 0000
 * Operation mask2 & ~REL_ATTR_COUNT_MASK clears the first 11 bits.
 * mask2 = 0101 1000 0000 0000
 * The next | fills 11 bits with a new value.
 */
#define SET_REL_ATTR_COUNT(rHd, relAttrCount) \
( \
    (rHd)->mask2 = ((rHd)->mask2 & ~REL_ATTR_COUNT_MASK) | (relAttrCount) \
)

typedef struct SRowPointer
{
	BlockIdData     block;
	uint16       pos;
} SRowPointer, *RowPointer;

typedef struct SRowFields
{
	uint          tranMin;		
	uint          tranMax;
	union
	{
		uint	  cmdId;		
		uint      tranVac;	
	}             field3;
} SRowFields, *RowFields;


typedef struct SDataFields
{
	uint		  len;
	uint		  typeMod;
	uint		  typeId;
} SDataFields, *DataFields;


typedef struct SRelRowHeader
{
	union
	{
		SRowFields   fields;
		SDataFields  data;
	}          typeData;

	SRowPointer      curr;		
	uint16		     mask2;	
	uint16		     mask;	
	uint8		     offset;
    uint8            nullBits[1]; /* null bits can be any number 8n */
} SRelRowHeader, *RelRowHeader;


#define RelRowSetCmdId(row, cmdId) \
( \
    (row)->data->typeData.fields.field3.cmdId = cmdId \
)

#define RelRowSetTranMin(row, tranId) \
( \
	(row)->data->typeData.fields.tranMin = tranId \
)

#define RelRowSetTranMax(row, tranId) \
( \
	(row)->data->typeData.fields.tranMax = tranId \
)

#define HeapTupleHasExternal(row) \
   (((row)->data->mask & ROW_HASEXTERNAL) != 0)

#define MAX_ROW_SIZE MaxRowSize_By_RowsPerPage(ROWS_PER_PAGE)

typedef struct SRelRow
{ 
    uint             len;
    SRowPointer      self; /* Pointer  */
    uint		 	 tblId;
    RelRowHeader     data;
} SRelRow, *RelRow;

/* If we have a row with 'attCount' number of attaributes.
 * And some of them are nulls. We determine how many 
 * bytes we need to keep the information about what attribues 
 * are nulls. 
 * If we have 8 attributes, we can use 1 char or 8 bits to store 
 * the nulls array: 1001 0010. If we have 9 attaributes we 
 * need 2 bytes. So the following formula is clear.
 */
#define BITMAP_LEN(attCount) (((int)(attCount) + 7) / 8)

/* int8* nullsArr
 * int   attNum
 * attNum >> 3 we delete our attNum by 8. 
 * For example we have a number 9 = 1001. 9 >> 3 = 1 = 9 / 8 = 1
 * 0x07 = 111. attNum & 0x07 is equivalent to attNum mod 8.
 */
#define IS_ATT_NULL(attNum, nullsArr) \
    (!((nullsArr)[(attNum) >> 3] & (1 << ((attNum) & 0x07))))

/* Determines if we can press an attribute. 
 * att->len = -1 indicates that the length is the same as 
 * the size of the attaribute's type.
 * stotageStrategy = 'o' indicates that the attribute is not put in
 * common, ordinary way.
 */
#define ATT_CAN_PACK(att) \
	((att)->len == -1 && (att)->storageStrategy != 'o')

/* each attribute has type
 * 'i' means integer and should be aligned as an integer value.
 * 'd' means double and should be aligned as a double value.
 * 's' means short and should be aligned as a short value.
 * 'o' means ordinary, plain. An attribute will be stored without 
 * alignment.
 */
#define ATT_ALIGN(offset, type) \
( \
	  ((type) == 'i') ? \
		  ALIGN_INT(offset) \
      : \
	  ( \
         ((type) == 'o') ? \
		    (int)(offset) \
         : \
	     ( \
            ((type) == 'd') ? \
               ALIGN_DOUBLE(offset) \
            : \
			   ALIGN_SHORT(offset) \
         ) \
      ) \
)

/* retrieves a value from a data pointer.
 */
#define ATT_RETRIEVE(data, byval, len) \
( \
	(byval) ? \
	( \
		(len) == (int)sizeof(int) ? \
		    (uint)SET_4_BYTES(*((int*)(data))) \
		: \
		( \
			(len) == (int)sizeof(int16) ? \
				(uint)SET_2_BYTES(*((int16*)(data))) \
			: \
		        (uint)SET_1_BYTE(*((char*)(data))) \
		) \
	) \
	: \
	(uint)((char*)(data)) \
)

#define SET_ATTR_VALUE(dataP,val,len) \
	do { \
		switch (len) \
		{ \
			case sizeof(char): \
				*(char*)(dataP) = (char)GET_1_BYTE(val); \
				break; \
			case sizeof(int16): \
				*(int16*)(dataP) = (int16)GET_2_BYTES(val); \
				break; \
			case sizeof(int): \
				*(int*)(dataP) = (int)GET_4_BYTES(val); \
				break; \
			default: \
				break; \
		} \
	} while (0)


/*
 * MaxHeapTupleSize is the maximum allowed size of a heap tuple, including
 * header and MAXALIGN alignment padding.  Basically it's BLCKSZ minus the
 * other stuff that has to be on a disk page.  Since heap pages use no
 * "special space", there's no deduction for that.
 *
 * NOTE: we allow for the ItemId that must point to the tuple, ensuring that
 * an otherwise-empty page can indeed hold a tuple of this size.  Because
 * ItemIds and tuples have different alignment requirements, don't assume that
 * you can, say, fit 2 tuples of size MaxHeapTupleSize/2 on the same page.
 */
#define MaxHeapTupleSize  (BLOCK_SIZE - ALIGN(SizeOfPageHeaderData + sizeof(ItemIdData)))

#define INVALID_TBLID 0

#endif