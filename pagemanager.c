#include "pagemanager.h"

void initializePage(
    void*         self,
	void*         page, 
	size_t        size, 
	size_t        suplSize)
{
	PageHeader	p = (PageHeader)page;

	suplSize = ALIGN_DEFAULT(suplSize);
    memset(p, 0, size);

	p->freeStart = SizeOfPageHeader;
	p->freeEnd   = size - suplSize;
	p->special   = size - suplSize;
}

size_t getFreeSpaceBase(
	  void*            self,
	  void*            page)
{
	int			space;
    PageHeader  phd = (PageHeader)page;

	space = phd->freeEnd - phd->freeStart;
	if (space < sizeof(SItemId))
       return 0;

	space -= sizeof(SItemId);

	return (size_t)space;
}

size_t getFreeSpace(
	  void*            self,
	  void*            page)
{
    int			space;
	uint16      nitems, item;

	space = getFreeSpaceBase(self, page);
	if (space <= 0)
		return space;
	
	/* Here space > 0. We calculate rows count */
    nitems = RowsCountOnPage(page);

	/* The number of rows on the page has not exceeded 
	 * the maximum number. We return the space.
	 */
	if (nitems < MaxRowCountPerPage)
		return space;

	/* Here the page is completely full and we try
	 * to find a free items. If a page's flag shows 
	 * that there are not free items, we return 0.
	 */
 	if (!PageHasFreeItems(page))
        return 0;

	/* Loop through all items and try to find an unuded one. */
	for (item = 1; item <= nitems; item++)
	{
		ItemId itid = GetItemIdFromPage(page, item);
        
		/* If item is not in use we break. We have found 
		 * an unused item. */
		if (!ItemIdIsInUse(itemId))
			break;	
    }

	/* We have not found a free item. Return 0. */
	if (item > nitems)
	    return 0;

	return space;
}

uint16 addItemToPage(
	void*          page,
    void*          item,
    size_t         size,
	uint16         itemnum,
    Bool           overwrite)
{
	ItemId		 itemId;
    PageHeader   pageHdr     = (PageHeader)page;
	uint16       maxItemNum  = RowsCountOnPage(page) + 1;
	Bool		 needShuffle = False;
	int          num;
	int			 newFreeStart, newFreeEnd;
	size_t		 alignedSize;

	if (IsItemIdValid(itemnum))
	{
        if (overwrite)
		{
			itemId = GetItemIdFromPage(page, itemnum);

			/* If the item is in use or has not empty storage
			 * we can not use it. Return 0.
			 */
            if (ItemIdIsInUse(itemId) || ItemIdHasStorage(itemId))
			    return 0;
		}
		else
		{
		     /* If we are not overwriting an item
		      * and the offset number in less than max number
			  * Our page needs shuffle to free a room for the item.
			  */
            if (itemnum < maxItemNum)
				needShuffle = True;
		}
	}
	else
	{
        /* If itemnum is not passed in or is invalid,
		 * we need to find a free item. If there does not exist
		 * a free item, we will put it at the end of the page.
		 */
		if (PageHasFreeItems(pageHdr))
		{
            /* Look for unused items */
			for (num = 1; num < maxItemNum; num++)
			{
				itemId = GetItemIdFromPage(page, itemnum);
                
				/* If the item is unused we break. */
				if (!ItemIdIsInUse(itemId) && !ItemIdHasStorage(itemId))
                    break;
			}

			/* If we have reached the maximum offset, there are not free items. */
			if (num >= maxItemNum)
			{
				/* the hint is wrong, so reset it */
                PageClearHasFreeLinePointers(page);
			}
		}
		else
		{
			/* If we have not a free item we try to insert it 
			 * into the end of a page. 
			 */
			itemnum = maxItemNum;
		}
	}

	/* If we are inserting an item into the end or with a shuffle
	 * we need to insert an item into items array. We pull the free start
	 * right on one position.
	 */
	if (itemnum == maxItemNum || needShuffle)
		newFreeStart = pageHdr->freeStart + sizeof(SItemId);
	else
		newFreeStart = pageHdr->freeStart;

    alignedSize = ALIGN_DEFAULT(size); 

	/* Calculate new free end. */
	newFreeEnd = (int)pageHdr->freeEnd - (int)alignedSize;

	/* if newFreeStart exceeds newFreeEnd we do not have 
	 * a free room to put the item to
	 */
	if (newFreeStart > newFreeEnd)
		return 0;

	itemId = GetItemIdFromPage(page, itemnum);

	if (needShuffle)
        memmove(itemId + 1, itemId, 
		        (maxItemNum - itemnum) * sizeof(SItemId));
    
	itemId->flags = ITEM_NORMAL;
	itemId->off   = newFreeEnd;
	itemId->len   = size;

    /* copy the item's data onto the page */
	memcpy((char*)page + newFreeEnd, item, size);

	/* Change free space range, freeStart and freeEnd. */
	pageHdr->freeStart = (uint16)newFreeStart;
	pageHdr->freeEnd   = (uint16)newFreeEnd;

	return itemnum;
}

