#ifndef BRIN_PAGE_H
#define BRIN_PAGE_H

#include "pageitempointer.h"

typedef struct BrinMetaPageData
{
	uint brinVersion;
	BlockNumber pagesPerRange;
	BlockNumber lastRangemapPage;
} BrinMetaPageData;

typedef struct RangemapContents
{
	PageItemPointerData mapIds[1];
} RangemapContents;

#define RangeMapContentSize \
	(BlockSize - SizeOfPageHeader - \
	 offsetof(RangemapContents, mapIds))

#define RangeMapPageMaxItems \
	(RangeMapContentSize / sizeof(ItemPointerData))
	
#endif