#ifndef BRIN_TUPLE_H
#define BRIN_TUPLE_H

#include "block.h"
#include "brin_internal.h"

typedef struct BrinTupleValues
{
	int16 attributeNumber;
	Bool hasNulls;
	Bool allNulls;
	Datum* values;
} BrinTupleValues;

typedef struct BrinMemoryTuple
{
    Bool isPlaceholderTuple;
	BlockNumber blockNumber;
	BrinTupleValues columnValues[FlexibleArrayMember];
} BrinMemoryTuple;

typedef struct BrinTuple
{
	BlockNumber block;

	/* ---------------
	 * data is laid out in the following fashion:
	 *
	 * 7th (high) bit: has nulls
	 * 6th bit: is placeholder tuple
	 * 5th bit: unused
	 * 4-0 bit: offset of data
	 * ---------------
	 */
	uint8 data;
} BrinTuple;

#define SizeOfBrinTuple (offsetof(BrinTuple, data) + sizeof(uint8))

#define BrinOffsetMask 0x1F // 0001 1111

#define BrinNullsMask 0x80 // 1000 0000

#define BrinTupleDataOffset(tuple) ((size_t)(((BrinTuple*)(tuple))->data & BrinOffsetMask))

#define BrinTupleHasNulls(tuple) (((((BrinTuple*)(tuple))->data & BrinNullsMask)) != 0)

BrinMemoryTuple* brinDeformTuple(BrinDescriptor* brinDescriptor, BrinTuple* brinTuple);

#endif