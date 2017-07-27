#include "common.h"
#include "index.h"
#include "catalog_index.h"
#include "rel.h"
#include "heap_operations.h"
#include "buffermanager.h"
#include "brin_range_map.h"
#include "brin_internal.h"

typedef struct BrinOpaque
{
	BlockNumber pagesPerRange;
	BrinRangemap* rangeMap;
	BrinDescriptor* brinDescriptor;
} BrinOpaque;

int getNumberOfBlocks(uint heapRelationId);

IndexScanDescriptor brinBeginScan(Relation relation, int numberOfKeys, int numberOfOrderBys)
{
	IndexScanDescriptor scanDescriptor = RelationGetIndexScan(relation, numberOfKeys, numberOfOrderBys);

    BrinRangemap* rangeMap = brinRangemapInitialize(relation);

    BrinDescriptor* brinDescriptor = brinBuildDescriptor(relation);

    BrinOpaque* opaque = (BrinOpaque*)malloc(sizeof(BrinOpaque));
    
    opaque->rangeMap = rangeMap;  

    opaque->brinDescriptor = brinDescriptor;

    scanDescriptor->opaque = opaque;

	return scanDescriptor; 
}

int64 brinScanTable(IndexScanDescriptor scanDescriptor)
{
	uint indexRelationId = RelationGetRelationId(scanDescriptor->indexRelation);

	uint heapRelationId = IndexGetRelation(indexRelationId);

    BlockNumber numberOfBlocks = getNumberOfBlocks(heapRelationId);

    BrinOpaque* opaque = (BrinOpaque*)scanDescriptor->opaque;

    int blockNumber;

    for (blockNumber = 0; blockNumber < numberOfBlocks; blockNumber += opaque->pagesPerRange)
    {
        BrinTupleInfo* brinTuple = brinGetTupleForHeapBlock(opaque->rangeMap, blockNumber);

        BrinMemoryTuple* brinMemoryTuple = brinDeformTuple(opaque->brinDescriptor, brinTuple);
    }

    return 0;
}

int getNumberOfBlocks(uint heapRelationId)
{
    Relation heapRelation = heapOpen(heapRelationId);

    BlockNumber numberOfBlocks = RelationGetNumberOfBlocks(heapRelation); 
    
    heapClose(heapRelationId); 

    return numberOfBlocks;
}


