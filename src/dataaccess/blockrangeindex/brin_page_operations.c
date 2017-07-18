#include "brin_page_operations.h"
#include "buffermanager.h"

Bool brinCanDoSamePageUpdate(BufferId buffer, size_t originalSize, size_t newSize);

Bool brinUpdateTuple(Relation indexRelation, BrinRangemap* rangemap, BlockNumber heapBlockNumber, BrinTupleUpdateRequest* updateRequest)
{
	Bool canUseSamePage = brinCanDoSamePageUpdate(updateRequest->oldBuffer, updateRequest->originalSize, updateRequest->newSize);
}

Bool brinCanDoSamePageUpdate(BufferId buffer, size_t originalSize, size_t newSize)
{
	size_t additionalSize = newSize - originalSize;

	Bool newSizeIsLessThanOriginalSize = newSize <= originalSize;

    Bool thereAreEnoughFreeSpace = PageGetExactFreeSpace(BufferGetPage(buffer)) >= additionalSize;

	return newSizeIsLessThanOriginalSize || thereAreEnoughFreeSpace;
}