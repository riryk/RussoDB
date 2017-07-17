#include "brin_internal.h"
#include "brin_range_map.h"
#include "brin_tuple.h"
#include "brin_page_operations.h"

BrinColumnDescriptor* brinGetColumnDescriptor(Relation relation, uint columnIndex);

BrinDescriptor* brinBuildDescriptor(Relation relation);

void setTupleValues(BrinMemoryTuple* brinMemoryTuple, Datum** values, Bool* nulls);

Bool brinInsert(Relation indexRelation, PageItemPointer heapTupleId, Datum** values, Bool* nulls)
{
	BrinRangemap* rangemap = brinRangemapInitialize(indexRelation);

    BlockNumber blockNumber = ItemPointerGetBlockNumber(heapTupleId);

    BlockNumber blockNumberNormalized = (blockNumber / rangemap->pagesPerRange) * rangemap->pagesPerRange;

    BrinTupleInfo* brinTupleInfo = brinGetTupleForHeapBlock(rangemap, blockNumberNormalized);

    BrinDescriptor* brinDescriptor = brinBuildDescriptor(indexRelation);

	BrinMemoryTuple* brinMemoryTuple = brinDeformTuple(brinDescriptor, brinTupleInfo->tuple);

    size_t originalSize = ItemIdGetLength(brinTupleInfo->pageItemId);

    BrinTuple* originalTuple = brinCopyTuple(brinTupleInfo->tuple, ItemIdGetLength(brinTupleInfo->pageItemId));

    setTupleValues(brinDescriptor, brinMemoryTuple, values, nulls);

    {
        FormBrinTupleResult* newDiskTupleResult = brinFormTuple(brinDescriptor, brinTupleInfo->blockNumber, brinMemoryTuple);

        Bool canUseSamePage = brinCanDoSamePageUpdate(brinTupleInfo->buffer, originalSize, newDiskTupleResult->size);
    }

	return False;
}

BrinDescriptor* brinBuildDescriptor(Relation relation)
{
	TupleDescriptor descriptor = RelationGetDescriptor(relation);

    BrinColumnDescriptor** columns = (BrinColumnDescriptor**)malloc(sizeof(BrinColumnDescriptor*) * descriptor->attributesNumber);

    int totalStored = 0;

    int columnIndex;

    for (columnIndex = 0; columnIndex < descriptor->attributesNumber; columnIndex++)
	{
        columns[columnIndex] = brinGetColumnDescriptor(relation, columnIndex);

		totalStored += columns[columnIndex]->totalStored;
	}

    {
    	int totalSize = offsetof(BrinDescriptor, columns) + sizeof(BrinColumnDescriptor*) * descriptor->attributesNumber;

    	BrinDescriptor* brinDescriptor = malloc(totalSize);

    	brinDescriptor->indexRelation = relation;
    	brinDescriptor->tupleDescriptor = descriptor;
    	brinDescriptor->totalStored = totalStored;
        
        for (columnIndex = 0; columnIndex < descriptor->attributesNumber; columnIndex++)
	    {
            brinDescriptor->columns[columnIndex] = columns[columnIndex];
	    }

	    free(columns);

	    return brinDescriptor;
    }
}

BrinColumnDescriptor* brinGetColumnDescriptor(Relation relation, uint columnIndex)
{
    // TODO: Form column descriptor for each indexed column
    return (BrinColumnDescriptor*)malloc(sizeof(BrinColumnDescriptor));
}

void setTupleValues(BrinDescriptor* brinDescriptor, BrinMemoryTuple* brinMemoryTuple, Datum** values, Bool* nulls)
{
    int attrIndex;

    for (attrIndex = 0; attrIndex < brinDescriptor->tupleDescriptor->attributesNumber; attrIndex++)
    {
        BrinTupleValues columnValues = brinMemoryTuple->columnValues[attrIndex]; 

        columnValues.values = values[attrIndex];

        columnValues.hasNulls = nulls[attrIndex];
    }
}
