#ifndef BRIN_INTERNAL_H
#define BRIN_INTERNAL_H

#include "rel.h"
#include "pageitempointer.h"
#include "tuple.h"

typedef struct BrinColumnDescriptor
{
    uint16 numberOfColumnsStored;
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

#endif