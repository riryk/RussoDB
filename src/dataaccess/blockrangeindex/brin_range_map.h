#ifndef BRIN_RANGE_MAP_H
#define BRIN_RANGE_MAP_H

#include "rel.h"
#include "block.h"
#include "buffer.h"
#include "offset.h"
#include "brin_tuple.h"

struct BrinRangemap
{
	Relation relation;
	BlockNumber pagesPerRange;
	BlockNumber lastRangemapPage;
	BufferId metadataBuffer;
	BufferId currentBuffer;
};

typedef struct BrinRangemap BrinRangemap;

struct BrinTupleInfo
{
    BrinTuple* tuple;
    BufferId buffer;
    OffsetNumber offsetNumber;
    BlockNumber blockNumber;
    ItemPointer pageItemId;
};

typedef struct BrinTupleInfo BrinTupleInfo;

extern BrinRangemap* brinRangemapInitialize(Relation indexRelation);

extern BrinTupleInfo* brinGetTupleForHeapBlock(BrinRangemap* rangemap, BlockNumber heapBlock);

#endif