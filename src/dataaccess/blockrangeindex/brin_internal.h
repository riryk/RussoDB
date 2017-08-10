#ifndef BRIN_INTERNAL_H
#define BRIN_INTERNAL_H

#include "common.h"
#include "rel.h"
#include "pageitempointer.h"
#include "tuple.h"
#include "typecache.h"
#include "buffer.h"

typedef struct BrinColumnDescriptor
{
    uint16 totalStored;
    TypeCacheEntry* typeCache[FlexibleArrayMember];
} BrinColumnDescriptor;

typedef struct BrinDescriptor
{
	Relation indexRelation;
	TupleDescriptor tupleDescriptor;
	TupleDescriptor diskTupleDescriptor;
	uint totalStored;
	BrinColumnDescriptor* columns[FlexibleArrayMember];
} BrinDescriptor;

extern Bool brinInsert(Relation indexRelation, PageItemPointer heapTupleId);

extern BrinDescriptor* brinBuildDescriptor(Relation relation);

#endif