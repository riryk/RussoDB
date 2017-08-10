#include "brin_page_operations.h"
#include "buffermanager.h"
#include "offset.h"

Bool brinCanDoSamePageUpdate(BufferId buffer, size_t originalSize, size_t newSize);

Bool brinUpdateTuple(Relation indexRelation, BrinRangemap* rangemap, BlockNumber heapBlockNumber, BrinTupleUpdateRequest* updateRequest)
{
	//TODO: Write update brin tuple on page
	Bool canUseSamePage = brinCanDoSamePageUpdate(updateRequest->oldBuffer, updateRequest->originalSize, updateRequest->newSize);
}

OffsetNumber brinInsertTuple(Relation indexRelation, BlockNumber pagesPerRange, BrinRangemap* rangemap, 
	BlockNumber heapBlock, BrinTuple* brinTuple, size_t brinTupleSize)
{
    //TODO: Write insert index tuple into index relation
    return 0;
}

Bool brinCanDoSamePageUpdate(BufferId buffer, size_t originalSize, size_t newSize)
{
	size_t additionalSize = newSize - originalSize;

	Bool newSizeIsLessThanOriginalSize = newSize <= originalSize;

    Bool thereAreEnoughFreeSpace = PageGetExactFreeSpace(BufferGetPage(buffer)) >= additionalSize;

	return newSizeIsLessThanOriginalSize || thereAreEnoughFreeSpace;
}