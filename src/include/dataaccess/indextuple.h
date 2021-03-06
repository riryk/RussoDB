#include "pageitempointer.h"

#ifndef Index_tuple_h
#define Index_tuple_h

typedef struct IndexTupleData
{
	PageItemPointerData PointerToTuple;		

	/* 
	 * 15th (high) bit:      has nulls
	 * 14th bit:             has variable width attributes
	 * 13th bit:             unused
	 * 12-0 bit:             size of tuple
	 */
	unsigned short IndexProperties;		

} IndexTupleData;

typedef IndexTupleData *IndexTuple;

#define IndexSizeMask 0x1FFF    
#define IndexHasVariableWidthAttributesMask 0x4000
#define IndexIsNullMask 0x8000

#define IndexTupleSize(tuple) ((size_t)(((IndexTuple)(tuple))->IndexProperties & IndexSizeMask))

#define IndexTupleSizeExceedsLimit(tupleSize) ((tupleSize & IndexSizeMask) != tupleSize)

#define SetDefaultValueToPointerToTuple(pointer) \
( \
	SetBlockIdTo(&((pointer)->BlockId), InvalidBlockNumber), \
	(pointer)->PositionId = InvalidPageItemOffset \
)

#endif





