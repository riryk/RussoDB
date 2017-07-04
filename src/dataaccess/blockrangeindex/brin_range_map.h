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

typedef struct BrinRangemap BrinRangemap;

extern BrinRangemap* brinRangemapInitialize(Relation indexRelation);

extern BrinTuple* brinGetTupleForHeapBlock(BrinRangemap* rangemap, BlockNumber heapBlock);

#endif