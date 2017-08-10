#include "brin_internal.h"
#include "rel.h"
#include "brin_tuple.h"
#include "brin_range_map.h"
#include "buffermanager.h"
#include "catalog_index.h"
#include "brin_page_operations.h"

typedef struct BrinBuildState
{
	Relation indexRelation;
	int numberOfTuples;
	int currentInsertBuffer;
	BlockNumber pagesPerRange;
	BlockNumber currentRangeStart;
	BrinRangemap* rangemap;
	BrinDescriptor* brinDescriptor;
	BrinMemoryTuple* brinMemoryTuple;
} BrinBuildState;

typedef OffsetNumber (*BrinInsertTupleFunc)(Relation, BlockNumber, BrinRangemap*, BlockNumber, BrinTuple*, size_t);

BrinInsertTupleFunc BrinInsertTuple = brinInsertTuple;

BrinBuildState* initializeBrinBuildState(Relation indexRelation, BrinRangemap* rangemap, BlockNumber pagesPerRange);

void brinBuildIndexCallback(Relation indexRelation, HeapTuple heapTuple, Datum* values, Bool* isnull,
	Bool tupleIsAlive, void* brinBuildState);

void brinSummarize(Relation indexRelation, Relation heapRelation)
{
    BrinRangemap* rangemap = brinRangemapInitialize(indexRelation);
    BlockNumber heapNumberOfBlocks = RelationGetNumberOfBlocks(heapRelation);
    BrinBuildState* brinBuildState = initializeBrinBuildState(indexRelation, rangemap, rangemap->pagesPerRange);
    IndexInfo* brinIndexInfo = BuildIndexInfo(indexRelation);

    int heapBlock = 0;
    for (heapBlock = 0; heapBlock < heapNumberOfBlocks; heapBlock += rangemap->pagesPerRange)
	{
        BrinTupleInfo* brinTuple = brinGetTupleForHeapBlock(rangemap, heapBlock);	
        Bool heapBlockIsSummarized = brinTuple != NULL;
        if (heapBlockIsSummarized)
        {
        	continue;
        }
	}     

	free(rangemap);
}

BrinBuildState* initializeBrinBuildState(Relation indexRelation, BrinRangemap* rangemap, BlockNumber pagesPerRange)
{
	BrinBuildState* brinBuildState = malloc(sizeof(BrinBuildState));
    
    brinBuildState->indexRelation = indexRelation;
    brinBuildState->numberOfTuples = 0;
    brinBuildState->currentInsertBuffer = InvalidBuffer;
    brinBuildState->pagesPerRange = pagesPerRange;
    brinBuildState->currentRangeStart = 0;
    brinBuildState->rangemap = rangemap;
    brinBuildState->brinDescriptor = brinBuildDescriptor(indexRelation);
	brinBuildState->brinMemoryTuple = brinNewMemoryTuple(brinBuildState->brinDescriptor);

    brinMemoryTupleInitialize(brinBuildState->brinMemoryTuple, brinBuildState->brinDescriptor);

	return brinBuildState;
}

void brinSummarizeRange(IndexInfo* brinIndexInfo, BrinBuildState* brinBuildState, Relation heapRelation,
	BlockNumber heapBlock, BlockNumber heapNumberOfBlocks)
{
	size_t brinPlaceHolderTupleSize;
    BrinMemoryTuple* brinPlaceHolderTuple = brinFormPlaceholderTuple(brinBuildState->brinDescriptor, heapBlock, &brinPlaceHolderTupleSize);

    OffsetNumber placeHolderTupleOffset = BrinInsertTuple(brinBuildState->indexRelation, brinBuildState->pagesPerRange, 
    	brinBuildState->rangemap, heapBlock, brinPlaceHolderTuple, brinPlaceHolderTupleSize);

    Bool isLastPageRange = heapBlock + brinBuildState->pagesPerRange <= heapNumberOfBlocks;

	BlockNumber numberOfBlocksToScan = isLastPageRange ? brinBuildState->pagesPerRange : heapNumberOfBlocks - heapBlock;

    Bool allowSyncronousScan = False;
    Bool scanAnyVisible = True;

	IndexBuildHeapRangeScan(heapRelation, brinBuildState->indexRelation, brinIndexInfo, allowSyncronousScan,
		scanAnyVisible, heapBlock, numberOfBlocksToScan, brinBuildIndexCallback, brinBuildState);
}

void brinBuildIndexCallback(Relation indexRelation, HeapTuple heapTuple, Datum* values, Bool* isnull,
	Bool tupleIsAlive, void* brinBuildState)
{
        
}