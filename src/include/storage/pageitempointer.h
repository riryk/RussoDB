
#include "common.h"
#include "block.h"

#ifndef Page_Item_Ppointer_h
#define Page_Item_Ppointer_h

typedef uint16 PageItemOffset;
typedef uint16 PageItemLength;

/*
 * This is a pointer to an item within a disk page of a known file
 * (for example, a cross-link from an index to its parent table).
 * blockid tells us which block, position tells us which entry in (BlockItemData) array we want.
 */
typedef struct PageItemPointerData
{
	BlockIdData BlockId;
	PageItemOffset PositionId;
}

/* If compiler understands packed and aligned pragmas, use those */
#if defined(pg_attribute_packed) && defined(pg_attribute_aligned)
pg_attribute_packed()
pg_attribute_aligned(2)
#endif
ItemPointerData;

#define SizeOfIptrData	\
	(offsetof(PageItemPointerData, PositionId) + sizeof(PageOffsetNumber))

typedef PageItemPointerData *PageItemPointer;

#endif