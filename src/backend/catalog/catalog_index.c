#include "catalog_index.h"

uint IndexGetRelation(uint indexId)
{
	//TODO: Get relation id having index id
    return 0;
}

IndexInfo* BuildIndexInfo(Relation indexRelation)
{
	return NULL;
}

double IndexBuildHeapRangeScan(Relation heapRelation, Relation indexRelation, IndexInfo* indexInfo,
	Bool allowSyncronousScan, Bool scanAnyVisible, BlockNumber startBlockNumber, BlockNumber endBlockNumber,
	IndexBuildCallback callback, void* callbackState)
{
	//TODO: Implement scan of some pages in the heap and call indexBuildCallback
	return 0;
}
