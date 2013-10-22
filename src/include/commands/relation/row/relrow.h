
#ifndef IRELROW_H
#define IRELROW_H

#include "common.h"


typedef struct SRowPointer
{
	SBlockId     block;
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
		SRowFields   fiels;
		SDataFields  data;
	}          typeData;

	SRowPointer      curr;		
	uint16		     mask2;	
	uint16		     mask;	
	uint8		     offset;
    uint8            nullBits;
} SRelRowHeader, *RelRowHeader;

typedef struct SRelRow
{ 
    uint             len;
    SRowPointer      self;
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
	((type) == 'i') ? ALIGN_INT(offset) : \
	 (((type) == 'o') ? (int)(offset) : \
	  (((type) == 'd') ? ALIGN_DOUBLE(offset) : ALIGN_SHORT(offset))) \
)

#endif