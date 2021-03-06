#ifndef Catalog_index_h
#define Catalog_index_h

#include "rel.h"
#include "block.h"
#include "tuple.h"

typedef struct IndexInfo
{
	Bool brokenHotChain;
} IndexInfo;

typedef void (*IndexBuildCallback)(Relation index, HeapTuple heapTuple, Datum* values, 
	Bool* isNull, Bool tupleIsAlive, void* state);

extern uint IndexGetRelation(uint indexId);

extern IndexInfo* BuildIndexInfo(Relation indexRelation);

extern double IndexBuildHeapRangeScan(Relation heapRelation, Relation indexRelation, IndexInfo* indexInfo,
	Bool allowSyncronousScan, Bool scanAnyVisible, BlockNumber startBlockNumber, BlockNumber endBlockNumber,
	IndexBuildCallback callback, void* callbackState);

#endif