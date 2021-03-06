#include "brin_range_map.h"
#include "buffermanager.h"
#include "page.h"
#include "brin_page.h"
#include "buffer.h"
#include "offset.h"

#define HeapBlockToRangeMapBlock(pagesPerRange, heapBlock) \
	((heapBlock / pagesPerRange) / RangeMapPageMaxItems)

#define HeapBlockToRangeMapIndex(pagesPerRange, heapBlock) \
	((heapBlock / pagesPerRange) % RangeMapPageMaxItems)

static void reloadRangeMapBuffer(BrinRangemap* rangemap, BlockNumber mapBlock);

static BlockNumber getRangeMapBlockForHeapBlock(BrinRangemap* rangemap, BlockNumber heapBlock);

BrinRangemap* brinRangemapInitialize(Relation indexRelation)
{
	BufferId metadataBuffer = ReadBuffer(indexRelation, 0);
	Page metadataPage = BufferGetPage(metadataBuffer);
	BrinMetaPageData* metadata = (BrinMetaPageData*)PageGetContents(metadataPage);

    BrinRangemap* rangeMap = (BrinRangemap*)malloc(sizeof(BrinRangemap));

    rangeMap->relation = indexRelation;
    rangeMap->pagesPerRange = metadata->pagesPerRange;
    rangeMap->lastRangemapPage = metadata->lastRangemapPage;
	rangeMap->metadataBuffer = metadataBuffer;

	return rangeMap;
}

BrinTupleInfo* brinGetTupleForHeapBlock(BrinRangemap* rangemap, BlockNumber heapBlock)
{
    BlockNumber rangeMapBlock = getRangeMapBlockForHeapBlock(rangemap, heapBlock);

	if (rangeMapBlock == InvalidBlockNumber)
	{
		return NULL;
	}

    reloadRangeMapBuffer(rangemap, rangeMapBlock);

    {
	    RangemapContents* rangemapContent = (RangemapContents*)PageGetContents(BufferGetPage(rangemap->currentBuffer));

        uint heapBlockMapIndex = HeapBlockToRangeMapIndex(rangemap->pagesPerRange, heapBlock);

        PageItemPointerData* heapBlockMapIds = rangemapContent->mapIds + heapBlockMapIndex;

        BlockNumber blockNumber = ItemPointerGetBlockNumber(heapBlockMapIds);

        OffsetNumber offsetNumber =  ItemPointerGetOffsetNumber(heapBlockMapIds);

		BufferId buffer = ReadBuffer(rangemap->relation, blockNumber);
		
		Page page = BufferGetPage(buffer);

		ItemPointer pageItemId = PageGetItemId(page, offsetNumber);

		BrinTuple* brinTuple = (BrinTuple*)PageGetItem(page, pageItemId);

        BrinTupleInfo* brinTupleInfo = (BrinTupleInfo*)malloc(sizeof(BrinTupleInfo));

        brinTupleInfo->tuple = brinTuple;

        brinTupleInfo->pageItemId = pageItemId;

        brinTupleInfo->blockNumber = blockNumber;

        brinTupleInfo->buffer = buffer;

		return brinTupleInfo;
    }
}

static void reloadRangeMapBuffer(BrinRangemap* rangemap, BlockNumber mapBlock) 
{
	if (rangemap->currentBuffer == InvalidBuffer)
	{
        rangemap->currentBuffer = ReadBuffer(rangemap->relation, mapBlock);
	}
	else if (BufferGetBlockNumber(rangemap->currentBuffer) != mapBlock)
	{
		ReleaseBuffer(rangemap->currentBuffer);

		rangemap->currentBuffer = ReadBuffer(rangemap->relation, mapBlock);
	}
}

static BlockNumber getRangeMapBlockForHeapBlock(BrinRangemap* rangemap, BlockNumber heapBlock)
{
	BlockNumber normalizedHeapBlock = (heapBlock / rangemap->pagesPerRange) * rangemap->pagesPerRange;

    // The first block is reserved for metadata. That's why we add 1. 
	BlockNumber rangeMapBlock = HeapBlockToRangeMapBlock(rangemap->pagesPerRange, normalizedHeapBlock) + 1;

    Bool rangeMapPageAlreadyAllocated = rangeMapBlock <= rangemap->lastRangemapPage; 

    return rangeMapPageAlreadyAllocated ? rangeMapBlock : InvalidBlockNumber;
}