#include "tuple_id_bitmap.h"
#include "hashtablemanager.h"

void tupleIdBitmapMarkPageLossy(TupleIdBitmap* tupleIdBitmap, BlockNumber pageNumber);

void tupleIdBitmapCreatePageTable(TupleIdBitmap* tupleIdBitmap);

void removePage(TupleIdBitmap* tupleIdBitmap, BlockNumber pageNumber);

void lookUpOrCreateChunkHeaderPage(TupleIdBitmap* tupleIdBitmap, BlockNumber chunkPageNumber);

void tupleIdBitmapLossify(TupleIdBitmap* tupleIdBitmap);

void tupleIdBitmapAddPage(TupleIdBitmap* tupleIdBitmap, BlockNumber blockNumber)
{
	Bool exceedsMaxAllowedEntries;

	tupleIdBitmapMarkPageLossy(tupleIdBitmap, blockNumber);

    exceedsMaxAllowedEntries = tupleIdBitmap->numberOfEntries > tupleIdBitmap->maxNumberOfEntries;

	if (exceedsMaxAllowedEntries)
	{
		tupleIdBitmapLossify(tupleIdBitmap);
	}
}

void tupleIdBitmapMarkPageLossy(TupleIdBitmap* tupleIdBitmap, BlockNumber pageNumber)
{
    if (tupleIdBitmap->status != TupleIdBitmapHash)
    {
        tupleIdBitmapCreatePageTable(tupleIdBitmap);
    }
    {
        int bitNumber = pageNumber % PagesPerChunk;
	    BlockNumber chunkPageNumber = pageNumber - bitNumber;
     
        if (bitNumber != 0)
        {
            removePage(tupleIdBitmap, pageNumber);
        }

        lookUpOrCreateChunkHeaderPage(tupleIdBitmap, chunkPageNumber);
    }
}

void removePage(TupleIdBitmap* tupleIdBitmap, BlockNumber pageNumber)
{
	Bool itemWasPresentBeforeRemoval = hashRemove(tupleIdBitmap->pageTable, (void*)&pageNumber) != NULL;
	if (itemWasPresentBeforeRemoval)
	{
        tupleIdBitmap->numberOfEntries--;
	    tupleIdBitmap->numberOfPages--;
	}
}

void lookUpOrCreateChunkHeaderPage(TupleIdBitmap* tupleIdBitmap, BlockNumber chunkPageNumber)
{
    Bool wasFoundBeforeInsert;
    PageTableEntry* page = hashInsert(tupleIdBitmap->pageTable, (void*)&chunkPageNumber, &wasFoundBeforeInsert);
    if (!wasFoundBeforeInsert)
	{
	    memset(page, 0, sizeof(PageTableEntry));

		page->blockNumber = chunkPageNumber;
		page->isChunk = True;

		tupleIdBitmap->numberOfEntries++;
		tupleIdBitmap->numberOfChunks++;
	}
	else if (!page->isChunk)
	{
		memset(page, 0, sizeof(PageTableEntry));

		page->blockNumber = chunkPageNumber;
		page->isChunk = True;

        tupleIdBitmap->numberOfPages--;
		tupleIdBitmap->numberOfChunks++;
	}
}

void tupleIdBitmapCreatePageTable(TupleIdBitmap* tupleIdBitmap)
{
	int hashTableSize = TupleIdBitmapHashtableStartSize;
    int hashTableFlags = HashElement | HashBlobs;

    SHashtableSettings settings;

	memset(&settings, 0, sizeof(HashtableSettings));
	settings.keyLen = sizeof(BlockNumber);
	settings.valLen = sizeof(PageTableEntry);

    tupleIdBitmap->pageTable = createHashtable("TupleIdBitmap", hashTableSize, &settings, hashTableFlags);     

    if (tupleIdBitmap->status == TupleIdBitmapOnePage)
	{
		Bool hashTableItemFound; 
		PageTableEntry* page = hashInsert(tupleIdBitmap->pageTable, tupleIdBitmap->onePageEntry.blockNumber, &hashTableItemFound);

		memcpy(page, &tupleIdBitmap->onePageEntry, sizeof(PageTableEntry));
	}

	tupleIdBitmap->status = TupleIdBitmapHash;
}

void tupleIdBitmapLossify(TupleIdBitmap* tupleIdBitmap)
{
    HashSequentialScanStatus scanStatus;
	PageTableEntry* page;
	Bool lossifiedEnough = False;

    hashSequentialScanInit(&scanStatus, tupleIdBitmap->pageTable);

	while ((page = (PageTableEntry*)hashSequentialSearch(&scanStatus)) != NULL && !lossifiedEnough)
	{
        if (page->isChunk || (page->blockNumber % PagesPerChunk == 0))
        {
			continue;	  
        }        

        tupleIdBitmapMarkPageLossy(tupleIdBitmap, page->blockNumber);       

        lossifiedEnough = tupleIdBitmap->numberOfEntries <= tupleIdBitmap->maxNumberOfEntries / 2;
	}

	hashSequentialSearchTerminate(&scanStatus);
}