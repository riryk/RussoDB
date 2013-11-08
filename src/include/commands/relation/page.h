
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

typedef struct SPageTLog
{
	uint		logid;
	uint		offset;
} SPageTLog, *PageTLog;

typedef struct SPageHeader
{
	SPageTLog	    log;	        
	uint16		    tLine;			
	uint16		    flags;	
	uint16          freeStart;
	uint16          freeEnd;
	uint16          special;	
	uint16		    version;
	uint            tranId;
	SItemId	        items[1];		
} SPageHeader, *PageHeader;

#define SizeOfPageHeader (offsetof(SPageHeader, items))

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

/* Consider that we put only one row per page.
 * We substract memory allocated for a page header 
 * and for ItemId.
 */
#define MaxRowSize_OneRowPerPage \
    (BLOCK_SIZE - ALIGN_DEFAULT(SizeOfPageHeader + sizeof(SItemId)))

/* If a row is larger than MAX_ROW_SIZE
 * we should compress compressable attributes and 
 * out external attributes to another place.
 */
#define ROWS_PER_PAGE	4

#define MAX_ROW_SIZE	MaxRowSize_By_RowsPerPage(ROWS_PER_PAGE)


#endif