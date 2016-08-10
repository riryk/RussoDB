
#include "block.h"

#ifndef Page_Item_Pointer_h
#define Page_Item_Pointer_h

#define InvalidPosition 0

typedef uint16 PageItemOffset;
typedef uint16 PageItemLength;

#define InvalidPageItemOffset ((PageItemOffset)0)

typedef struct PageItemPointerData
{
	BlockIdData BlockId;
	PageItemOffset PositionId;
}

#if defined(pg_attribute_packed) && defined(pg_attribute_aligned)
pg_attribute_packed()
pg_attribute_aligned(2)
#endif

PageItemPointerData;

#define SizeOfIptrData	\
	(offsetof(PageItemPointerData, PositionId) + sizeof(PageOffsetNumber))

typedef PageItemPointerData *PageItemPointer;

#define SetItemPointerTo(pointer, blockNumber, offsetNumber) \
( \
	SetBlockIdTo(&((pointer)->BlockId), blockNumber), \
	(pointer)->PositionId = offsetNumber \
)

#endif