#ifndef BRIN_INTERNAL_H
#define BRIN_INTERNAL_H

#include "common.h"
#include "rel.h"
#include "pageitempointer.h"
#include "tuple.h"
#include "typecache.h"
#include "buffer.h"
#include "brin.h"
#include "brin_tuple.h"
#include "brin_range_map.h"

typedef OffsetNumber (*BrinInsertTupleFunc)(Relation, BlockNumber, BrinRangemap*, BlockNumber, BrinTuple*, size_t);

Bool addAndCompareTuple(BrinDescriptor* brinDescriptor, BrinTupleValues* brinTupleColumnValues, Datum* values, Bool* nulls);

extern Bool brinInsert(Relation indexRelation, PageItemPointer heapTupleId);

extern BrinDescriptor* brinBuildDescriptor(Relation relation);

#endif