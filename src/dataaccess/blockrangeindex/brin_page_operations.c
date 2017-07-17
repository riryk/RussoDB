#include "brin_page_operations.h"
#include "buffermanager.h"

Bool brinCanDoSamePageUpdate(BufferId buffer, size_t originalSize, size_t newSize)
{
	size_t additionalSize = newSize - originalSize;

	Bool newSizeIsLessThanOriginalSize = newSize <= originalSize;

    Bool thereAreEnoughFreeSpace = PageGetExactFreeSpace(BufferGetPage(buffer)) >= additionalSize;

	return newSizeIsLessThanOriginalSize || thereAreEnoughFreeSpace;
}

Bool brinUpdateTuple(Relation indexRelation, BlockNumber pagesPerRange,
			         BrinRangemap* rangemap, BlockNumber heapBlockNumber,
			         BufferId oldBuffer, OffsetNumber oldBufferOffset,
			         const BrinTuple *origtup, Size origsz,
			         const BrinTuple *newtup, Size newsz,
			         bool samepage)