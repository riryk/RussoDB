#ifndef Tuple_id_bitmap_h
#define Tuple_id_bitmap_h

#include "block.h"
#include "common.h"
#include "hashtable.h"
#include "bitmapset.h"
#include "page.h"

#define PagesPerChunk (BlockSize / 32)

#define MaxTuplesPerPage MaxRowCountPerPage

#define WordsPerPage ((MaxTuplesPerPage - 1) / BitsPerBitmapword + 1)

#define WordsPerChunk ((PagesPerChunk - 1) / BitsPerBitmapword + 1)

typedef enum
{
	TupleIdBitmapEmpty, /* No hashtable, numberOfEntries == 0 */
	TupleIdBitmapOnePage, /* First entry contains the single entry */
	TupleIdBitmapHash /* PageTable is valid, entry1 is not */
} TupleIdBitmapStatus;

typedef struct PageTableEntry
{
	BlockNumber blockNumber; /* page number (hashtable key) */
	Bool isChunk;
	Bool tupledNeedRecheck;
	bitmapword words[Max(WordsPerPage, WordsPerChunk)];
} PageTableEntry;

typedef struct TupleIdBitmap
{
	TupleIdBitmapStatus status;		
	Hashtable pageTable;
	int numberOfEntries;
	int maxNumberOfEntries;
	int numberOfPages;
	int numberOfChunks;
	Bool iterating;
	PageTableEntry onePageEntry;
} TupleIdBitmap;

#define TupleIdBitmapHashtableStartSize 128

void tupleIdBitmapAddPage(TupleIdBitmap* tupleIdBitmap, BlockNumber blockNumber);

#endif