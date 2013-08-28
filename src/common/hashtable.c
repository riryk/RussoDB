
#include "hashtable.h"

int my_log2(long num)
{
	int			i;
	long		limit;

	for (i = 0, limit = 1; limit < num; i++, limit <<= 1)
		;
	return i;
}

Hashtable createHashtable(
	char*              name, 
	long               maxItemsNum, 
	HashtableSettings  set, 
	int                setFlags)
{
	Hashtable    tbl;
	int          nbucks;
	int          nsegs;
    int          nbucksRounded;
	long         ffactor;        
	int          segSize = 256;
    int          dirSize = 256;
	HASHSEGMENT  *segp;

	/* Initialize the hash header, plus a copy of the table name.
	 * First sizeof(Hashtable) bytes are allocated for a hash table.
	 * The next strlen(Name) are allocated for the name.
	 * In this way we unate one malloc call for struct SHashtable with another 
	 * malloc for char* name.
	 */
	tbl = (Hashtable)memAlloc(sizeof(SHashtable) + strlen(name) + 1);
	memset(tbl, 0, sizeof(SHashtable));
	tbl->name = (char*)(tbl + 1);

    tbl->hashFunc = hashFuncStr;
	if (setFlags & HASH_FUNC)
		tbl->hashFunc = set->hashFunc;
	
    tbl->hashCmp = memcmp;
	if (tbl->hashFunc == hashFuncStr)
		tbl->hashCmp = hashCmpFuncStr;

	if (setFlags & HASH_CMP)
		tbl->hashCmp = set->hashCmp;

	tbl->isInShared = False;
	tbl->noInserts = True;
	tbl->keyLen = set->keyLen;

    if (setFlags & HASH_SEG)
	{
		tbl->segmSize = set->segmSize;
		tbl->segmShift = set->segmShift;
	}
    
	/* If maxItemsNum = 400; nbucks = (400 - 1) / 1 + 1  = 400 
	 * my_log2(400) = 512 the next greater power of two
	 */
	ffactor = 1;
	nbucks = (maxItemsNum - 1) / ffactor + 1;
	nbucksRounded = 1 << my_log2(lnbuckets);
    
	/* max_bucket = 511, high_mask = 1024 - 1 = 1023 */
	tbl->max_bucket = tbl->low_mask = nbucksRounded - 1;
	tbl->high_mask = (nbucksRounded << 1) - 1;

	/* nbucksRounded = 512, segSize = 256 
	 * nsegs = 2
	 */
	nsegs = (nbucksRounded - 1) / segSize + 1;

	/* Allocate memory for directory array (array of segments) */
	tbl->dir = (HASHSEGMENT*)malloc(dirSize * sizeof(HASHSEGMENT));
    
    /* Allocate initial segments */
	for (segp = tbl->dir; tbl->nsegs < nsegs; hctl->nsegs++, segp++)
	{
		*segp = (HASHSEGMENT)malloc(sizeof(HASHBUCKET) * segSize);
	    MemSet(*segp, 0, sizeof(HASHBUCKET) * hashp->ssize);
	}
    
	{
		int	      nelem_alloc;
		uint      elementSize;
		uint      allocSize;

		/* Each element has a HASHELEMENT header plus user data. */
	    /* NB: this had better match element_alloc() */
	    elementSize = MAXALIGN(sizeof(HASHELEMENT)) + MAXALIGN(entrysize);
		/*
	     * The idea here is to choose nelem_alloc at least 32, but round up so
	     * that the allocation request will be a power of 2 or just less. For hash
	     * tables managed by palloc, the allocation request will be rounded up to
	     * a power of 2 anyway.  If we fail to take this into account, we'll waste
	     * as much as half the allocated space.
	     */
	    allocSize = 32 * 4;			/* assume elementSize at least 8 */

		/* Test for elementSize = 8
		 * allocSize = 32 * 4 * 2 
		 * nelem_alloc = 32 * 4 * 2 / 8 = 32
		 *
		 * If elementSize = 16
		 * allocSize = 32 * 4 * 2 
		 * nelem_alloc = 32 * 4 * 2 / 16 = 16
		 */
	    do
	    {
		   allocSize <<= 1;
		   nelem_alloc = allocSize / elementSize;
	    } 
		while (nelem_alloc < 32);
	}

	return tbl;
}

static void SequenceScanInit(struct Hashtable* Table)
{
	if (SequenceScansCount >= SEQUENCE_MAX_SCANS)
		;//Error("The maximum count of opened hash search scans exceeded: %d", SequenceScansCount);

	SequenceScans[SequenceScansCount] = Table;
	SequenceScansCount++;
}

void HashSequenceInit(struct HashSequenceItem* SeqItem, Hashtable Table)
{
	/*SeqItem->HashTableHeader = Table;*/
	SeqItem->CurrentBucket = 0;
	SeqItem->CurrentItem = NULL;
    
	if (!Table->noEnlarge)
		SequenceScanInit(Table);
}

void* HashSequenceSearch(struct HashSequenceItem* SeqItem)
{
	Hashtable                hashTable;
	struct HashtableHeader*  hashTableHeader; 
	unsigned int             maxBucketId;
	long		             segmentSize;
	long		             segmentNum;
	long		             segmentIndex;
	struct HashItem*         currentSegment;
	unsigned int             currentBucket;
	struct HashItem*         currentItem;

	if ((currentItem = SeqItem->CurrentItem) != NULL)
	{
		SeqItem->CurrentItem = currentItem->Next;
		if (SeqItem->CurrentItem == NULL)	
			SeqItem->CurrentBucket++; /* Go to the next bucket */
           
	    /*  10 + 7 =  000010001  & 111111000 = 16
		 *  23 + 7 =  000011110  & 111111000 = 11000 = 
		 *  ((LEN + 7) & ~7) - round it to 8*k
		 */

		//return (void*)((char*)currentItem) + (sizeof(HashItem) + 7) & ~7);
	}

	/*
	 * Search for next nonempty bucket starting at curBucket.
	 */
	currentBucket = SeqItem->CurrentBucket;
	hashTable = SeqItem->Table->header;
	hashTableHeader = hashTable->header;
	segmentSize = hashTable->segmSize;
	maxBucketId = hashTableHeader->MaxBucketId;

	if (currentBucket > maxBucketId)
	{
		return NULL;			
	}

	segmentNum = currentBucket >> hashTable->segmShift;
	//segmentIndex = MOD(currentBucket, segmentSize);
	currentSegment = hashTable->startSegm[segmentNum];

	while ((currentItem = &currentSegment[segmentIndex]) == NULL)
	{
		if (++currentBucket > maxBucketId)
		{
			SeqItem->CurrentBucket = currentBucket;
			return NULL;
		}
		if (++segmentIndex >= segmentSize)
		{
			segmentNum++;
			segmentIndex = 0;
			currentSegment = hashTable->startSegm[segmentNum];;
		}
	}

	SeqItem->CurrentItem = currentItem->Next;
	if (SeqItem->CurrentItem == NULL)	
		++currentBucket;
	SeqItem->CurrentBucket = currentBucket;
	//return (void*)((char*)currentItem) + ((sizeof(struct HashItem) + 7) & ~7);
	return (void*)"asdasda";
}

static int HasSequenceScans(struct Hashtable* hashTable)
{
	int			i;

	for (i = 0; i < SequenceScansCount; i++)
	{
		if (SequenceScans[i] == hashTable)
			return 1;
	}
	return 0;
}

static struct HashItem** AllocateNewSegment(Hashtable hashTable)
{
	struct HashItem** newSegmentHeader;

	newSegmentHeader = (struct HashItem**)hashTable->hashAlloc(sizeof(struct HashItem*) * hashTable->segmSize);

	if (!newSegmentHeader)
		return NULL;

	memset(newSegmentHeader, 0, sizeof(struct HashItem*) * hashTable->segmSize);

	return newSegmentHeader;
}

static int DirectoryRealloc(Hashtable hashTable)
{
	long		NewSize;
	long        OldDirectorySize;
	long        NewDirectorySize;
	struct HashItem*   newDir;
	struct HashItem*   oldDir;

	if (hashTable->header->MaxDirectorySize != -1)
		return 0;
    /* Multiple for 2 */
	NewSize = hashTable->header->DirectorySize << 1; 

	OldDirectorySize = hashTable->header->DirectorySize * sizeof(struct HashItem);
	NewDirectorySize = NewSize * sizeof(struct HashItem);

	oldDir = hashTable->dir; 
	newDir = (struct HashItem*)hashTable->hashAlloc(NewDirectorySize);

	if (newDir != NULL)
	{
		memcpy(newDir, oldDir, OldDirectorySize);
		memset(((char*)newDir) + OldDirectorySize, 0, NewDirectorySize - OldDirectorySize);
		hashTable->dir = newDir;
		hashTable->header->DirectorySize = NewSize;
		free(oldDir);
		return 1;
	}
	return 0;
}

/* Convert a hash value to a bucket number */
static unsigned int ConvertHashToBuketId(struct HashtableHeader* tableHeader, unsigned int hash)
{
	unsigned int		bucketId;

	bucketId = hash & tableHeader->HighMask;
	if (bucketId > tableHeader->MaxBucketId)
		bucketId = bucketId & tableHeader->LowMask;

	return bucketId;
}

static int ExpandTable(Hashtable hashTable)
{
	struct HashtableHeader* tableHeader = hashTable->header;
	long            OldBucket;
	long			NewBucket;
    long            OldSegmentNumber;
	long            NewSegmentNumber;
    long            OldSegmentIndex;
	long            NewSegmentIndex;
	struct HashItem**      OldSegment;
    struct HashItem**      NewSegment;
	struct HashItem**      OldLink;
    struct HashItem**      NewLink;
	struct HashItem*       CurrentElem;
	struct HashItem*       NextElem;

    NewBucket = tableHeader->MaxBucketId + 1;
	/* We need explanation
	 * Suppose that NewBucket = 53. (110101)
	 * We have segment size as power of 2: 2^3 = 8
	 * SegmentShift will be 3. 110101 >> 3 = 110 = 6.  6*8 = 48 + 5
	 * 6 is segments count. We have 6 full segments 
	 * 8 - 1 = 000111
	 * 110101 & = 101 = 5
	 * 000111
	 */ 
	NewSegmentNumber = NewBucket >> 2; // SegmentShift;
	NewSegmentIndex = NewBucket & (hashTable->segmSize - 1);

	if (NewSegmentNumber >= tableHeader->SegmentsCount)
	{
        /* In this case we need to allocate new memory for a new segment */
		if (NewSegmentNumber >= tableHeader->DirectorySize) 
           if (!DirectoryRealloc(hashTable))
				return 0;
        if (!(hashTable->dir[NewSegmentNumber] = AllocateNewSegment(hashTable)))
		   return 0;
        tableHeader->SegmentsCount++;
	}
    tableHeader->MaxBucketId++;

	/*  */
	OldBucket = NewBucket & tableHeader->LowMask;

	/* Let low mask be: 12 = 1100
	 *     high mask  : 45 = 101101
	 *     new bucket : 47 = 101111
	 * lowMask = 1100
	 * highMask = 101111 | 1100 = 
	 * highMask = 111 = 8
	 * newBucket = 9 => lowMask = 8
	 * highmask = 1001 | 11
	 */
    if (NewBucket > tableHeader->HighMask) 
	{
        tableHeader->LowMask = tableHeader->HighMask;
		tableHeader->HighMask = NewBucket | tableHeader->LowMask;
	}

	OldSegmentNumber = OldBucket >> 2; //SegmentShift;
	OldSegmentIndex = OldBucket & (hashTable->segmSize - 1);

	OldSegment = hashTable->dir[OldSegmentNumber];
	NewSegment = hashTable->dir[NewSegmentNumber];

	OldLink = &OldSegment[OldSegmentIndex];
	NewLink = &NewSegment[NewSegmentIndex];

	for (CurrentElem = *OldLink;
		 CurrentElem != NULL;
		 CurrentElem = NextElem)
	{
		NextElem = CurrentElem->Next;
		
		if (ConvertHashToBuketId(tableHeader, CurrentElem->Hash) == OldBucket)
		{
			*OldLink = CurrentElem;
			OldLink = &NextElem;
		}
		else
		{
			*NewLink = CurrentElem;
			NewLink = &NextElem;
		}
	}

	*OldLink = NULL;
	*NewLink = NULL;

	return 1;
}

static int AllocateNewItems(Hashtable hashTable, int elemCount)
{
    volatile struct HashtableHeader* tableHeader = hashTable->header;
	long                      headerSize;
	long                      dataSize;
    long		              size;
    struct HashItem*                 newItem;
	struct HashItem*                 prevItem;
	struct HashItem*                 currentItem;
	int                       i;

	if (hashTable->noEnlarge)
		return 0;

    headerSize = ((int)sizeof(struct HashItem) + 7) & ~7;
	dataSize = (tableHeader->DataItemSize + 7) & ~7;

	size = headerSize + dataSize;
    /* Here we newItem will contain a memory for all elements */
	newItem = (struct HashItem*)hashTable->hashAlloc(elemCount * size);
    if (!newItem)
		return 0;

    /* prepare to link all the new entries into the freelist */
	prevItem = NULL;
	currentItem = newItem;
	for (i = 0; i < elemCount; i++)
	{
		currentItem->Next = prevItem;
		prevItem = currentItem;
		currentItem = (struct HashItem*)(((char*)currentItem) + size);
	}

	if (hashTable->partNum > 0)
        SpinLockAcquire(&tableHeader->Locker, __FILE__, __LINE__);
     
	newItem->Next = tableHeader->FreeList; 
    tableHeader->FreeList = prevItem;

	if (hashTable->partNum > 0)
        SpinLockRelease(&tableHeader->Locker);

	return 1;
}

static struct HashItem* CreateNewItem(Hashtable hashTable)
{
    volatile struct HashtableHeader* tableHeader = hashTable->header;
	struct HashItem*	              newItem;

	for (;;)
	{
		/* If the table is partitioned among multiple threads
		 * let's analyze what can happen if we do not use spin lock
		 * two simultaneous threads can receive the same reference 
		 * and data where this reference points to will be corrupted
		 */
		if (hashTable->partNum > 0)
            SpinLockAcquire(&tableHeader->Locker, __FILE__, __LINE__);

		/* try to get an entry from the freelist */
		newItem = tableHeader->FreeList;
		if (newItem != NULL)
			break;

        if (hashTable->partNum > 0)
            SpinLockRelease(&tableHeader->Locker);

		/* If we are here - the free list empty.
		 * We allocate new memory to the free list and then 
		 * start loop again */
		if (!AllocateNewItems(hashTable, tableHeader->ItemsNumToAllocAtOnce))
		    return NULL;
	}
	/* We have read the first head item from the free list
	 * we move the header to one position next */
	tableHeader->FreeList = newItem->Next;
	tableHeader->ItemsNumber++;

	if (hashTable->partNum > 0)
        SpinLockRelease(&tableHeader->Locker);

	return newItem;
}

void* HashSearch(
		  Hashtable hashTable, 
		  void* key,
		  EHashAction action,
		  int* found)
{
	unsigned int    bucketId;
	struct HashItem**      Segment;
    struct HashItem**      BucketPointer;
	long		    SegmentNumber;
	long		    SegmentIndex;
	struct HashtableHeader* tableHeader = hashTable->header;
	unsigned int    hashValue = hashTable->hashFunc(key, hashTable->keyLen);

    /* During insertion we need to check if it is reasonable to split
	 * a hash table segment. */
	if (action == 0 /*HASH_ENTER*/ || action == 1 /*HASH_ENTER_NULL*/)
	{
        /* When we are not allowed to split table:
         * 1. When hash table is in partition mode
		 * 2. When hash table is non enlargable
		 * 3. table is the subject of any active hash_seq_search scans.  
		 * 4. The average bucket size exceeds the allowed bucket size 
		 */
		if (hashTable->partNum == 0 && 
			!hashTable->noEnlarge && 
			!HasSequenceScans(hashTable) && 
			tableHeader->ItemsNumber / (long) (tableHeader->MaxBucketId + 1) >= tableHeader->BucketSize)
		{
            ExpandTable(hashTable);
		}
	}

	bucketId = ConvertHashToBuketId(&tableHeader, hashValue);

	SegmentNumber = bucketId >> hashTable->segmShift;
	SegmentIndex = bucketId & (hashTable->segmSize - 1);

	Segment = hashTable->dir[SegmentNumber];
	if (Segment == NULL)
		;//Error("Hash table is corrupted");

	BucketPointer = &Segment[SegmentIndex];
	while (*BucketPointer != NULL)
	{
		void* elemKey = (void*)(((char*)(*BucketPointer)) + ((sizeof(struct HashItem) + 7) & ~7));

		if ((*BucketPointer)->Hash == hashValue &&
			hashTable->hashCmp(elemKey, key, hashTable->keyLen) == 0)
			break;
		BucketPointer = &((*BucketPointer)->Next);
	}

	if (found)
		*found = (*BucketPointer) != NULL;

	switch (action)
	{
	    case 1 /*HASH_ENTER*/:
		    if (*BucketPointer != NULL)
			    return (void*)(((char*)(*BucketPointer)) + ((sizeof(struct HashItem) + 7) & ~7));

		    if (hashTable->noEnlarge)
			    ;//Error("Can't enlarge");

	        *BucketPointer = CreateNewItem(hashTable);
			if (*BucketPointer == NULL)
			    ;//Error("Out of memory");

			(*BucketPointer)->Next = NULL;
			(*BucketPointer)->Hash = hashValue;

			/*hashp->keycopy(
				(void*)((char*)(*BucketPointer)) + ((sizeof(HashItem) + 7) & ~7), 
				key, 
				hashTable->KeyLength);*/

			//return (void*)((char*)(*BucketPointer)) + ((sizeof(struct HashItem) + 7) & ~7);
			return (void*)"asdasda";
	}
}