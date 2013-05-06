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
