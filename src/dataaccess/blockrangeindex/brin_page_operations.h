#ifndef Brin_Page_Operations_h
#define Brin_Page_Operations_h

#include "buffer.h"
#include "offset.h"
#include "brin_tuple.h"
#include "brin_range_map.h"

typedef struct BrinTupleUpdateRequest
{
	BufferId oldBuffer;
	OffsetNumber oldBufferOffset;

    BrinTuple* originalTuple;
    size_t originalSize;

	BrinTuple* newTuple;
	size_t newSize;
} BrinTupleUpdateRequest;

Bool brinUpdateTuple(Relation indexRelation, BrinRangemap* rangemap, BlockNumber heapBlockNumber, BrinTupleUpdateRequest* updateRequest);

OffsetNumber brinInsertTuple(Relation indexRelation, BlockNumber pagesPerRange, BrinRangemap* rangemap, 
	BlockNumber heapBlock, BrinTuple* brinTuple, size_t brinTupleSize);

#endif