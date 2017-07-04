#include "common.h"

#ifndef ITEMID_H
#define ITEMID_H

typedef struct ItemPointerData
{
	unsigned	off:15,		
				flags:2,    
				len:15;		
} ItemPointerData, *ItemPointer;


#define ITEM_UNUSED		0		/* unused when item's len = 0 */
#define ITEM_NORMAL		1		/* normal items are items with len > 0 */
#define ITEM_REDIRECT	2		/* len = 0 */
#define ITEM_DEAD		3		/* dead, may or may not have storage */

/* This macro checks if a item is in use or not */
#define ItemIdIsInUse(itemId) \
	((itemId)->flags != ITEM_UNUSED)

#define ItemIdHasStorage(itemId) \
	((itemId)->len != 0)

#define ItemIdGetOffset(itemId) \
   ((itemId)->off)

#endif
