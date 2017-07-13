#include "brin_internal.h"
#include "brin_range_map.h"

BrinColumnDescriptor* brinGetColumnDescriptor(Relation relation, uint columnIndex);

BrinDescriptor* brinBuildDescriptor(Relation relation);

Bool brinInsert(Relation indexRelation, PageItemPointer heapTupleId)
{
	BrinRangemap* rangemap = brinRangemapInitialize(indexRelation);

    BlockNumber blockNumber = ItemPointerGetBlockNumber(heapTupleId);

    BlockNumber blockNumberNormalized = (blockNumber / rangemap->pagesPerRange) * rangemap->pagesPerRange;

    BrinTuple* brinTuple = brinGetTupleForHeapBlock(rangemap, blockNumber);

    BrinDescriptor* brinDescriptor = brinBuildDescriptor(indexRelation);

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
    return (BrinColumnDescriptor*)malloc(sizeof(BrinColumnDescriptor));
}
