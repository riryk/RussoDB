#include <hashtable.h>

unsigned int HashSimple(void* Key, DWORD KeySize)
{
	return -1;
}

unsigned int HashString(void* Key, DWORD KeySize)
{
	return -1;
}

int StringCmp(char* Key1, char* Key2, DWORD KeySize)
{
	return strncmp(Key1, Key2, KeySize - 1);
}

unsigned int TagHash(void* Key, DWORD KeySize)
{
	return -1;
}

HashTable* HashTableCreate(
	char* Name, 
	long MaxItemsNum, 
	HashTableSettings* Info, 
	int Flags)
{
	HashTable*    hashTableHeader;
	//HASHHDR       *hctl

	/* Initialize the hash header, plus a copy of the table name */
	hashTableHeader = (HashTable*)malloc(sizeof(HashTable) + strlen(Name) + 1);
	/* First sizeof(HashTable) bytes are allocated for a hash table 
	 * structure. The next strlen(Name) is allocated for the name. */
	MemSet(hashTableHeader, 0, sizeof(HashTable));
    /* Set memory allocated for Name */
	hashTableHeader->Name = (char*)(hashTableHeader + 1);
	strcpy(hashTableHeader->Name, Name);

	if (Flags & HASH_FUNCTION)
		hashTableHeader->HashFunc = Info->HashFunc;
	else
		hashTableHeader->HashFunc = HashString;	

	if (Flags & HASH_COMPARE)
		hashTableHeader->HashCompare = Info->HashCompare;
	else if (hashTableHeader->HashFunc == HashString)
		hashTableHeader->HashFunc = (HashCompareFunc)StringCmp;
	else
		hashTableHeader->HashFunc = memcmp;

	if (Flags & HASH_KEYCOPY)
		hashTableHeader->HashCopy = Info->HashCopy;
	else if (hashTableHeader->HashFunc == HashString)
		hashTableHeader->HashCopy = (HashCopyFunc)strncpy;
	else
		hashTableHeader->HashCopy = memcpy;

	hashTableHeader->IsInShared = 0;
	hashTableHeader->NoMoreInserts = 0;

	hashTableHeader->KeyLength = Info->KeyLength;
    hashTableHeader->SegmentSize = Info->SegmentSize;
	hashTableHeader->SegmentShift = Info->SegmentShift;

	return hashTableHeader;
}

static void SequenceScanInit(Hashtable* Table)
{
	if (SequenceScansCount >= SEQUENCE_MAX_SCANS)
		Error("The maximum count of opened hash search scans exceeded: %d", SequenceScansCount);

	SequenceScans[SequenceScansCount] = Table;
	SequenceScansCount++;
}

void HashSequenceInit(HashSequenceItem* SeqItem, Hashtable* Table)
{
	SeqItem->HashTableHeader = Table;
	SeqItem->CurrentBucket = 0;
	SeqItem->CurrentItem = NULL;
    
	if (!Table->NoMoreInserts)
		SequenceScanInit(Table);
}

void* HashSequenceSearch(HashSequenceItem* SeqItem)
{
	Hashtable*        hashTable;
	HashtableHeader*  hashTableHeader; 
	unsigned int      maxBucketId;
	long		      segmentSize;
	long		      segmentNum;
	long		      segmentIndex;
	HashSegment       currentSegment;
	unsigned int      currentBucket;
	HashItem*         currentItem;

	if ((currentItem = SeqItem->CurrentItem) != NULL)
	{
		SeqItem->CurrentItem = currentItem->Next;
		if (SeqItem->CurrentItem == NULL)	
			SeqItem->CurrentBucket++; /* Go to the next bucket */
           
	    /*  10 + 7 =  000010001  & 111111000 = 16
		 *  23 + 7 =  000011110  & 111111000 = 11000 = 
		 *  ((LEN + 7) & ~7) - round it to 8*k
		 */
		return (void*)((char*)currentItem) + (sizeof(HashItem) + 7) & ~7);
	}

	/*
	 * Search for next nonempty bucket starting at curBucket.
	 */
	currentBucket = SeqItem->CurrentBucket;
	hashTable = SeqItem->HashTableHeader;
	hashTableHeader = hashTable->Header;
	segmentSize = hashTable->SegmentSize;
	maxBucketId = hashTableHeader->MaxBucketId

	if (currentBucket > maxBucketId)
	{
		return NULL;			
	}

	segmentNum = currentBucket >> hashTable->SegmentShift;
	segmentIndex = MOD(currentBucket, segmentSize);
	currentSegment = hashTable->StartSegment[segmentNum];

	while ((currentItem = currentSegment[segmentIndex]) == NULL)
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
			currentSegment = hashTable->StartSegment[segmentNum];;
		}
	}

	SeqItem->CurrentItem = currentItem->Next;
	if (SeqItem->CurrentItem == NULL)	
		++currentBucket;
	SeqItem->CurrentBucket = currentBucket;
	return return (void*)((char*)currentItem) + (sizeof(HashItem) + 7) & ~7);;
}

static int HasSequenceScans(Hashtable* hashTable)
{
	int			i;

	for (i = 0; i < SequenceScansCount; i++)
	{
		if (SequenceScans[i] == hashTable)
			return 1;
	}
	return 0;
}

static HashItem** AllocateNewSegment(Hashtable* hashTable)
{
	HashItem** newSegmentHeader;

	newSegmentHeader = (HashItem**)hashTable->HashAlloc(sizeof(HashItem*) * hashTable->SegmentSize);

	if (!newSegmentHeader)
		return NULL;

	MemSet(newSegmentHeader, 0, sizeof(HashItem*) * hashTable->SegmentSize);

	return newSegmentHeader;
}

static int DirectoryRealloc(Hashtable* hashTable)
{
	long		NewSize;
	long        OldDirectorySize;
	long        NewDirectorySize;
	HashItem*   newDir;
	HashItem*   oldDir;

	if (hashTable->Header->MaxDirectorySize != -1)
		return 0;
    /* Multiple for 2 */
	NewSize = hashTable->Header->DirectorySize << 1; 

	OldDirectorySize = hashTable->Header->DirectorySize * sizeof(HashItem);
	NewDirectorySize = NewSize * sizeof(HashItem);

	oldDir = hashTable->Directory; 
	newDir = (HashItem*)hashTable->HashAlloc(NewDirectorySize);

	if (newDir != NULL)
	{
		memcpy(newDir, oldDir, OldDirectorySize);
		MemSet(((char*)newDir) + OldDirectorySize, 0, NewDirectorySize - OldDirectorySize);
		hashTable->Directory = newDir;
		hashTable->Header->DirectorySize = NewSize;
		free(oldDir);
		return 1;
	}
	return 0;
}

/* Convert a hash value to a bucket number */
static unsigned int ConvertHashToBuketId(HashtableHeader* tableHeader, unsigned int hash)
{
	unsigned int		bucketId;

	bucketId = hash_val & tableHeader->HighMask;
	if (bucketId > tableHeader->MaxBucketId)
		bucketId = bucketId & tableHeader->LowMask;

	return bucketId;
}

static int ExpandTable(Hashtable* hashTable)
{
	HashtableHeader tableHeader = hashTable->Header;
	long            OldBucket;
	long			NewBucket;
    long            OldSegmentNumber;
	long            NewSegmentNumber;
    long            OldSegmentIndex;
	long            NewSegmentIndex;
	HashItem**      OldSegment;
    HashItem**      NewSegment;
	HashItem**      OldLink;
    HashItem**      NewLink;
	HashItem*       CurrentElem;
	HashItem*       NextElem;

    NewBucket = hashTable->MaxBucketId + 1;
	/* We need explanation
	 * Suppose that NewBucket = 53. (110101)
	 * We have segment size as power of 2: 2^3 = 8
	 * SegmentShift will be 3. 110101 >> 3 = 110 = 6.  6*8 = 48 + 5
	 * 6 is segments count. We have 6 full segments 
	 * 8 - 1 = 000111
	 * 110101 & = 101 = 5
	 * 000111
	 */ 
	NewSegmentNumber = NewBucket >> SegmentShift;
	NewSegmentIndex = NewBucket & (hashTable->SegmentSize - 1);

	if (NewSegmentNumber >= tableHeader->SegmentsCount)
	{
        /* In this case we need to allocate new memory for a new segment */
		if (NewSegmentNumber >= tableHeader->DirectorySize) 
           if (!DirectoryRealloc(hashTable))
				return 0;
        if (!(tableHeader->Directory[NewSegmentNumber] = AllocateNewSegment(hashTable)))
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

	OldSegmentNumber = OldBucket >> SegmentShift;
	OldSegmentIndex = OldBucket & (hashTable->SegmentSize - 1);

	OldSegment = hashTable->Directory[OldSegmentNumber];
	NewSegment = hashTable->Directory[NewSegmentNumber];

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

	return true;
}

static int AllocateNewItems(Hashtable* hashTable, int elemCount)
{
    volatile HashtableHeader* tableHeader = hashTable->Header;
	long                      headerSize;
	long                      dataSize;
    long		              size;
    HashItem*                 newItem;
	HashItem*                 prevItem;
	HashItem*                 currentItem;
	int                       i;

	if (hashTable->NotEnlarge)
		return 0;

    headerSize = ((int)sizeof(HashItem) + 7) & ~7;
	dataSize = (tableHeader->DataItemSize + 7) & ~7;

	size = headerSize + dataSize;
    /* Here we newItem will contain a memory for all elements */
	newItem = (HashItem*)hashTable->HashAlloc(elemCount * size);
    if (!newItem)
		return 0;

    /* prepare to link all the new entries into the freelist */
	prevElement = NULL;
	currentElement = newItem;
	for (i = 0; i < elemCount; i++)
	{
		currentElement->Next = prevElement;
		prevElement = currentElement;
		currentElement = (HashItem*)(((char*)currentElement) + size);
	}

	if (hashTable->PartitionNumber > 0)
        SpinLockAcquire(&tableHeader->Locker, __FILE__, __LINE__);
     
	newItem->Next = tableHeader->FreeList; 
    tableHeader->FreeList = prevElement;

	if (hashTable->PartitionNumber > 0)
        SpinLockRelease(&tableHeader->Locker);

	return true;
}

static HashItem* CreateNewItem(Hashtable* hashTable)
{
    volatile HashtableHeader* tableHeader = hashTable->Header;
	HashItem*	              newItem;

	for (;;)
	{
		/* If the table is partitioned among multiple threads
		 * let's analyze what can happen if we do not use spin lock
		 * two simultaneous threads can receive the same reference 
		 * and data where this reference points to will be corrupted
		 */
		if (hashTable->PartitionNumber > 0)
            SpinLockAcquire(&tableHeader->Locker, __FILE__, __LINE__);

		/* try to get an entry from the freelist */
		newItem = tableHeader->FreeList;
		if (newItem != NULL)
			break;

        if (hashTable->PartitionNumber > 0)
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

	if (hashTable->PartitionNumber > 0)
        SpinLockRelease(&tableHeader->Locker);

	return newElement;
}

void* HashSearch(
		  Hashtable* hashTable, 
		  void* key,
		  EHashAction action,
		  int* found)
{
	unsigned int    bucketId;
	HashItem**      Segment;
    HashItem**      BucketPointer;
	long		    SegmentNumber;
	long		    SegmentIndex;
	HashtableHeader tableHeader = hashTable->Header;
	unsigned int    hashValue = hashTable->HashFunc(key, hashTable->KeyLength);

    /* During insertion we need to check if it is reasonable to split
	 * a hash table segment. */
	if (action == HASH_ENTER || action == HASH_ENTER_NULL)
	{
        /* When we are not allowed to split table:
         * 1. When hash table is in partition mode
		 * 2. When hash table is non enlargable
		 * 3. table is the subject of any active hash_seq_search scans.  
		 * 4. The average bucket size exceeds the allowed bucket size 
		 */
		if (hashtable->PartitionNumber == 0 && 
			!hashtable->NotEnlarge && 
			!HasSequenceScans(hashTable) && 
			tableHeader->ItemsNumber / (long) (tableHeader->MaxBucketId + 1) >= tableHeader->BucketSize)
		{
            ExpandTable(hashtable);
		}
	}

	bucketId = ConvertHashToBuketId(tableHeader, hashValue);

	SegmentNumber = bucketId >> hashTable->SegmentShift;
	SegmentIndex = bucketId & (hashTable->SegmentSize - 1);

	Segment = hashTable->Directory[SegmentNumber];
	if (Segment == NULL)
		Error("Hash table is corrupted");

	BucketPointer = &Segment[SegmentIndex];
	while (*BucketPointer != NULL)
	{
		void* elemKey = (void*)(((char*)(*BucketPointer)) + (sizeof(HashItem) + 7) & ~7);

		if ((*BucketPointer)->Hash == hashValue &&
			hashTable->HashCompare(elemKey, key, hashTable->KeyLength) == 0)
			break;
		BucketPointer = &((*BucketPointer)->Next);
	}

	if (found)
		*found = (*BucketPointer) != NULL;

	switch (action)
	{
	    case HASH_ENTER:
		    if (*BucketPointer != NULL)
			    return (void*)(((char*)(*BucketPointer)) + (sizeof(HashItem) + 7) & ~7);

		    if (hashTable->NotEnlarge)
			    Error("Can't enlarge");

	        *BucketPointer = CreateNewItem(hashTable);
			if (*BucketPointer == NULL)
			    Error("Out of memory");

			(*BucketPointer)->Next = NULL;
			(*BucketPointer)->Hash = hashValue;

			hashp->keycopy(
				(void*)((char*)(*BucketPointer)) + (sizeof(HashItem) + 7) & ~7), 
				key, 
				hashTable->KeyLength);

			return (void*)((char*)(*BucketPointer)) + (sizeof(HashItem) + 7) & ~7);
	}
}