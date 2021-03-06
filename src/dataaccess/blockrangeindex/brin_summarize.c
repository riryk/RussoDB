#include "brin_internal.h"
#include "rel.h"
#include "brin_tuple.h"
#include "brin_range_map.h"
#include "buffermanager.h"
#include "catalog_index.h"
#include "brin_page_operations.h"
#include "pageitempointer.h"

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

BrinInsertTupleFunc BrinInsertTuple = brinInsertTuple;

BrinBuildState* initializeBrinBuildState(Relation indexRelation, BrinRangemap* rangemap, BlockNumber pagesPerRange);

void scanHeapRangeAndSumarizeBrinTuple(IndexInfo* brinIndexInfo, BrinBuildState* brinBuildState, Relation heapRelation,
    BlockNumber heapBlock, BlockNumber numberOfBlocksToScan);

void brinBuildIndexCallback(Relation indexRelation, HeapTuple heapTuple, Datum* values, Bool* isnull,
	Bool tupleIsAlive, void* brinBuildState);

void formAndInsertTuple(BrinBuildState* state);

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
        	continue;
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
    int placeHolderTupleBuffer;

    OffsetNumber placeHolderTupleOffset = BrinInsertTuple(brinBuildState->indexRelation, brinBuildState->pagesPerRange, 
    	brinBuildState->rangemap, &placeHolderTupleBuffer, heapBlock, brinPlaceHolderTuple, brinPlaceHolderTupleSize);

    Bool isLastPageRange = heapBlock + brinBuildState->pagesPerRange <= heapNumberOfBlocks;

	BlockNumber numberOfBlocksToScan = isLastPageRange ? brinBuildState->pagesPerRange : heapNumberOfBlocks - heapBlock;

    scanHeapRangeAndSumarizeBrinTuple(brinIndexInfo, brinBuildState, heapRelation, heapBlock, numberOfBlocksToScan);

    for (;;)
    {
		/*
        FormBrinTupleResult* tupleResult = brinFormTuple(state->brinDescriptor, heapBlock, brinBuildState->brinMemoryTuple); 

        Bool wasTupleUpdated = brinUpdateTuple(brinBuildState->indexRelation, brinBuildState->rangemap, heapBlock, placeHolderTupleBuffer,
            placeHolderTupleOffset, brinPlaceHolderTuple, brinPlaceHolderTupleSize, tupleResult->tuple, tupleResult->size);   

        free(brinPlaceHolderTuple);
        free(tupleResult);   

        if (wasTupleUpdated)
            break;

        brinPlaceHolderTuple = brinGetTupleForHeapBlock(brinBuildState->rangemap, heapBlock);
		*/
        //union_tuples(state->bs_bdesc, state->bs_dtuple, phtup);
    }
}

void scanHeapRangeAndSumarizeBrinTuple(IndexInfo* brinIndexInfo, BrinBuildState* brinBuildState, Relation heapRelation,
    BlockNumber heapBlock, BlockNumber numberOfBlocksToScan)
{
    Bool allowSyncronousScan = False;
    Bool scanAnyVisible = True;

    brinBuildState->currentRangeStart = heapBlock;

    IndexBuildHeapRangeScan(heapRelation, brinBuildState->indexRelation, brinIndexInfo, allowSyncronousScan,
        scanAnyVisible, heapBlock, numberOfBlocksToScan, brinBuildIndexCallback, brinBuildState);
} 

void brinBuildIndexCallback(Relation indexRelation, HeapTuple heapTuple, Datum* values, Bool* isnull,
	Bool tupleIsAlive, void* state)
{
    BrinBuildState* brinBuildState = (BrinBuildState*)state;
    BlockNumber tupleBlock = ItemPointerGetBlockNumber(&heapTuple->selfItemPointer);

    Bool isInFutureBlock = tupleBlock > brinBuildState->currentRangeStart + brinBuildState->pagesPerRange - 1;
    if (isInFutureBlock)
    {
        formAndInsertTuple(brinBuildState);
        brinBuildState->currentRangeStart += brinBuildState->pagesPerRange;
        brinMemoryTupleInitialize(brinBuildState->brinMemoryTuple, brinBuildState->brinDescriptor);
    }
    
    addAndCompareTuple(brinBuildState->brinDescriptor, brinBuildState->brinMemoryTuple->columnValues, values, isnull);     
}

void formAndInsertTuple(BrinBuildState* state)
{
    FormBrinTupleResult* tupleResult = brinFormTuple(state->brinDescriptor, state->currentRangeStart, state->brinMemoryTuple); 
    
    brinInsertTuple(state->indexRelation, state->pagesPerRange, state->rangemap, &state->currentInsertBuffer,
       &tupleResult->tuple, tupleResult->size);

    state->numberOfTuples++;

    free(tupleResult);
}

void brinUnionTuples(BrinDescriptor* brinDescriptor, BrinMemoryTuple* memoryTupleFirst, BrinTuple* tupleSecond)
{
    // FormBrinTupleResult* secondTupleResult = brinFormTuple(brinDescriptor, tupleSecond);
}

void brinInclusionUnion(BrinDescriptor* brinDescriptor, BrinTupleValues* firstColumn, BrinTupleValues* secondColumn);