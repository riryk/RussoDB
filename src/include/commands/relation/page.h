
#ifndef PAGE_H
#define PAGE_H

#include "common.h"
#include "stddef.h"

/*
 * +----------------+---------------------------------+
 * | PageHeader     | item1 item2 item3 ...			  |
 * +-----------+----+---------------------------------+
 * | ... itemN |									  |
 * +-----------+--------------------------------------+
 * |		   ^ freeStart							  |
 * |												  |
 * |			 v freeEnd							  |
 * +-------------+------------------------------------+
 * |			 | rowN ...  						  |
 * +-------------+------------------+-----------------+
 * |	   ... row3 row2 row1       | "special space" |
 * +--------------------------------+-----------------+
 *									^ special
 */
typedef struct SItemId
{
	unsigned	off:15,		
				flags:2,    
				len:15;		
} SItemId, *ItemId;


#define ITEM_UNUSED		0		/* unused when item's len = 0 */
#define ITEM_NORMAL		1		/* normal items are items with len > 0 */
#define ITEM_REDIRECT	2		/* len = 0 */
#define ITEM_DEAD		3		/* dead, may or may not have storage */

/* This macro checks if a item is in use or not */
#define ItemIdIsInUse(itemId) \
	((itemId)->flags != ITEM_UNUSED)

#define ItemIdHasStorage(itemId) \
	((itemId)->len != 0)

typedef struct SPageTLog
{
	uint		logid;
	uint		offset;
} SPageTLog, *PageTLog;

typedef struct SPageHeader
{
	SPageTLog	    log;	        
	uint16		    tLine;			
	uint16		    flags;	      /* flag bits */
	uint16          freeStart;    /* offset to start of free space */  
	uint16          freeEnd;      /* offset to end of free space */
	uint16          special;	  /* offset to start of special space */
	uint16		    version;
	uint            tranId;
	SItemId	        items[1];		
} SPageHeader, *PageHeader;

#define SizeOfPageHeader (offsetof(SPageHeader, items))

/* when there are any unused items */
#define PAGE_HAS_FREE_ITEMS	        0x0001		

/* there are not enough free space on a page */
#define PAGE_IS_FULL		        0x0002		

/* all items on a page are visible to everyone */
#define PAGE_ALL_ITEMS_ARE_VISIBLE	0x0004		

/* all flags are valid */
#define PAGE_ALL_FLAGS_ARE_VALID	0x0007		

/* This macros calculates the space which is needed for 
 * page header and items array. 
 */
#define SizeOfHeaderWithItemsArr(rowsPerPage) \
    (ALIGN_DEFAULT(SizeOfPageHeader + (rowsPerPage) * sizeof(SItemId)))

/* Suppose that we want to have 'rowsPerPage' rows on a page
 * BLOCK_SIZE is the total size of a page and 
 * BLOCK_SIZE - SizeOfHeaderWithItemsArr is the total size which can
 * be allocated for rows. Using this information we can easily calculate
 * maxRowSize.
 */
#define MaxRowSize_By_RowsPerPage(rowsPerPage) \
	(ALIGN_DOWN_DEFAULT((BLOCK_SIZE - SizeOfHeaderWithItemsArr(rowsPerPage)) / (rowsPerPage)))

/* This macro caclulates the maximum number of rows
 * which can fit into a page.
 */
#define MaxRowCountPerPage \
	((int) \
      ( \  
        (BLOCK_SIZE - SizeOfPageHeader) / \
		( \
           ALIGN_DEFAULT(offsetof(SRelRowHeader, nullBits)) + \
		   sizeof(SItemId) \
		) \   
      ) \
	) \

/* This macros determines if there is 
 * at least one free item on a page. 
 */
#define PageHasFreeItems(page) \
	(((PageHeader)(page))->flags & PAGE_HAS_FREE_ITEMS)

/* This macros clears PAGE_HAS_FREE_ITEMS flag 
 * from page's flags property.
 */
#define PageClearHasFreeLinePointers(page) \
	(((PageHeader)(page))->flags &= ~PAGE_HAS_FREE_ITEMS)

/* Get ItemId from page by number */
#define GetItemIdFromPage(page, num) \
    ((ItemId)(&((PageHeader)(page))->items[(num) - 1]))

/* Consider that we put only one row per page.
 * We substract memory allocated for a page header 
 * and for ItemId.
 */
#define MaxRowSize_OneRowPerPage \
    (BLOCK_SIZE - ALIGN_DEFAULT(SizeOfPageHeader + sizeof(SItemId)))

/* This macros calculates a number of items 
 * which are put on a page. 
 */
#define RowsCountOnPage(page) \
    ( \ 	
      ((PageHeader)(page))->freeStart <= SizeOfPageHeader ? \
      0 : \
      (((PageHeader)(page))->freeStart - SizeOfPageHeader) / sizeof(SItemId) \
	) \ 

#define MaxItemId  ((int)(BLOCK_SIZE / sizeof(SItemId)))

#define IsItemIdValid(itemId) \
	((Bool)((itemId != 0) && (itemId <= MaxItemId)))

/* If a row is larger than MAX_ROW_SIZE
 * we should compress compressable attributes and 
 * out external attributes to another place.
 */
#define ROWS_PER_PAGE	4

#define MAX_ROW_SIZE	MaxRowSize_By_RowsPerPage(ROWS_PER_PAGE)


#endif