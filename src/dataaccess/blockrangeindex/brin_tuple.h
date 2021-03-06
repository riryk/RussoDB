#ifndef BRIN_TUPLE_H
#define BRIN_TUPLE_H

#include "block.h"
#include "brin.h"

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

typedef struct FormBrinTupleResult
{
    BrinTuple tuple;
    size_t size;
} FormBrinTupleResult;

#define SizeOfBrinTuple (offsetof(BrinTuple, data) + sizeof(uint8))

#define BrinOffsetMask 0x1F // 0001 1111

#define BrinNullsMask 0x80 // 1000 0000

#define BrinTupleDataOffset(tuple) ((size_t)(((BrinTuple*)(tuple))->data & BrinOffsetMask))

#define BrinTupleHasNulls(tuple) (((((BrinTuple*)(tuple))->data & BrinNullsMask)) != 0)

void brinMemoryTupleInitialize(BrinMemoryTuple* brinTuple, BrinDescriptor* brinDescriptor);

BrinTuple* brinFormPlaceholderTuple(BrinDescriptor* brinDescriptor, BlockNumber block, size_t* tupleSizeComputed);

BrinTuple* brinCopyTuple(BrinTuple* tuple, size_t length);

FormBrinTupleResult* brinFormTuple(BrinDescriptor* brinDescriptor, BlockNumber block, BrinMemoryTuple* tuple);

BrinMemoryTuple* brinDeformTuple(BrinDescriptor* brinDescriptor, BrinTuple* brinTuple);

#endif