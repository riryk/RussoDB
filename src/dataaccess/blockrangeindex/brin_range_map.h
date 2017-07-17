#ifndef BRIN_RANGE_MAP_H
#define BRIN_RANGE_MAP_H

#include "rel.h"
#include "block.h"
#include "buffer.h"
#include "brin_tuple.h"

struct BrinRangemap
{
	Relation relation;
	BlockNumber pagesPerRange;
	BlockNumber lastRangemapPage;
	BufferId metadataBuffer;
	BufferId currentBuffer;
};

struct BrinTupleInfo
{
    BrinTuple* tuple;
    BufferId buffer;
    BlockNumber blockNumber;
    ItemPointer pageItemId;
};

typedef struct BrinRangemap BrinRangemap;

typedef struct BrinTupleInfo BrinTupleInfo;

extern BrinRangemap* brinRangemapInitialize(Relation indexRelation);

extern BrinTupleInfo* brinGetTupleForHeapBlock(BrinRangemap* rangemap, BlockNumber heapBlock);

#endif