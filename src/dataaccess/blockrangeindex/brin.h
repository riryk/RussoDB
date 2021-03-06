#ifndef BRIN_H
#define BRIN_H

#include "common.h"
#include "tuple.h"
#include "typecache.h"
#include "rel.h"

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

#endif