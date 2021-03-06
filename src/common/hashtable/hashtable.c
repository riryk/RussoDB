
#include "hashtable.h"

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
	/*SeqItem->CurrentBucket = 0;
	SeqItem->CurrentItem = NULL;
    
	if (!Table->noEnlarge)
		SequenceScanInit(Table);*/
}

void* HashSequenceSearch(struct HashSequenceItem* SeqItem)
{
	Hashtable                hashTable;
	struct HashtableHeader*  hashTableHeader; 
	unsigned int             maxBucketId;
	long		             segmentSize;
	long		             segmentNum;
	long		             segmentIndex;
	HashItem                 currentSegment;
	unsigned int             currentBucket;
	HashItem                 currentItem;

	if ((currentItem = SeqItem->CurrentItem) != NULL)
	{
		SeqItem->CurrentItem = currentItem->next;
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

	SeqItem->CurrentItem = currentItem->next;
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

