#include "common.h"
#include "index.h"
#include "catalog_index.h"
#include "rel.h"
#include "heap_operations.h"
#include "buffermanager.h"
#include "brin_range_map.h"
#include "brin_internal.h"
#include "tuple_id_bitmap.h"

typedef struct BrinOpaque
{
	BlockNumber pagesPerRange;
	BrinRangemap* rangeMap;
	BrinDescriptor* brinDescriptor;
} BrinOpaque;

int getNumberOfBlocks(uint heapRelationId);

Bool checkPageRangeToMatchScanKeys(IndexScanDescriptor scanDescriptor);

void addPageRangeToOutputBitmap(TupleIdBitmap* tuplesToScan, int* totalPages);

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

int brinScanTable(IndexScanDescriptor scanDescriptor, TupleIdBitmap* tuplesToScan)
{
	uint indexRelationId = RelationGetRelationId(scanDescriptor->indexRelation);

	uint heapRelationId = IndexGetRelation(indexRelationId);

    BlockNumber numberOfBlocks = getNumberOfBlocks(heapRelationId);

    BrinOpaque* opaque = (BrinOpaque*)scanDescriptor->opaque;

    int totalPages = 0;

    int blockNumber;

    for (blockNumber = 0; blockNumber < numberOfBlocks; blockNumber += opaque->pagesPerRange)
    {
        BrinTupleInfo* brinTuple = brinGetTupleForHeapBlock(opaque->rangeMap, blockNumber);

        BrinMemoryTuple* brinMemoryTuple = brinDeformTuple(opaque->brinDescriptor, brinTuple);

        Bool pageRangeNeedsToBeAddedToScanResult = checkPageRangeToMatchScanKeys(opaque->brinDescriptor, scanDescriptor, brinMemoryTuple);

        if (pageRangeNeedsToBeAddedToScanResult) 
        {
            addPageRangeToOutputBitmap(tuplesToScan, blockNumber, opaque->pagesPerRange, &totalPages);
        }
    }

    return totalPages;
}

Bool checkPageRangeToMatchScanKeys(BrinDescriptor* brinDescriptor, IndexScanDescriptor scanDescriptor, BrinMemoryTuple* brinMemoryTuple)
{
    int scanKeyNumber;

    for (scanKeyNumber = 0; scanKeyNumber < scanDescriptor->numberOfKeys; scanKeyNumber++)
    {
        ScanKey scanKey = &scanDescriptor->keyData[scanKeyNumber];
        uint scanKeyAttrNumber = scanKey->attributeNumber;
        BrinTupleValues* brinValues = &brinMemoryTuple->columnValues[scanKeyAttrNumber - 1];

        Bool scanKeyConsistentWithRangeValues = brinInclusionConsistent(brinDescriptor, brinValues, scanKey);
        if (!scanKeyConsistentWithRangeValues)
        {
            return False;
        }
    }
    
    return True;
}

void addPageRangeToOutputBitmap(TupleIdBitmap* tuplesToScan, BlockNumber startBlock, BlockNumber pagesPerRange, int* totalPages)
{
    int pageNumber;

    for (pageNumber = startBlock; pageNumber <= startBlock + pagesPerRange - 1; pageNumber++)
    {
        tupleIdBitmapAddPage(tuplesToScan, pageNumber);      

        (*totalPages)++;          
    }
}

int getNumberOfBlocks(uint heapRelationId)
{
    Relation heapRelation = heapOpen(heapRelationId);

    BlockNumber numberOfBlocks = RelationGetNumberOfBlocks(heapRelation); 
    
    heapClose(heapRelationId); 

    return numberOfBlocks;
}


