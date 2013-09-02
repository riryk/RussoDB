
#include "hashtablemanager.h"

const SIHashtableManager sHashtableManager = 
{ 
	&sTrackMemManager,
	createHashtable 
};

const IHashtableManager hashtableManager = &sHashtableManager;

Hashtable createHashtable(
    void*              self,
	char*              name, 
	long               maxItemsNum, 
	HashtableSettings  set, 
	int                setFlags)
{
	IHashtableManager _ = (IHashtableManager)self;
	Hashtable         tbl;
	int               nHashLists;
	int               nSegs;
	int               segLLSize;
	uint              elemSize;
    HashSegment*      segP; 

	/* Initialize the hash header, plus a copy of the table name.
	 * First sizeof(Hashtable) bytes are allocated for a hash table.
	 * The next strlen(Name) are allocated for the name.
	 * In this way we unate one malloc call for struct SHashtable with another 
	 * malloc for char* name.
	 */
	tbl = (Hashtable)_->memManager->alloc(sizeof(SHashtable) + strlen(name) + 1);
	memset(tbl, 0, sizeof(SHashtable));

	/* Adding one to a pointer means "produce a pointer to the object 
	 * that comes in memory right after this one," 
	 * which means that the compiler automatically scales up 
	 * whatever you're incrementing the pointer with 
	 * by the size of the object being pointed at. */
	tbl->name = (char*)(tbl + 1);
	strcpy(tbl->name, name);

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

	tbl->keyLen = sizeof(char *);
	tbl->valLen = 2 * tbl->keyLen;

	if (setFlags & HASH_ITEM)
	{
	    tbl->keyLen = set->keyLen;
	    tbl->valLen = set->valLen;
	}

    tbl->segmSize = DEFAULT_SEG_SIZE;
    tbl->segmShift = DEFAULT_SEG_SHIFT;

    if (setFlags & HASH_SEG)
	{
		tbl->segmSize = set->segmSize;
		tbl->segmShift = set->segmShift;
	}

	tbl->hashListSize = DEFAULT_HASH_LIST_SIZE;
    tbl->segmsAmount = DEFAULT_SEGS_AMOUNT;
	tbl->maxSegmsAmount = NO_SEGS_AMOUNT_RESTRICTIONS;

    if (setFlags & HASH_LIST_SIZE)
	{
		tbl->segmsAmount = set->segmsAmount;
		tbl->maxSegmsAmount = set->maxSegmsAmount;
		tbl->hashListSize = set->hashListSize;
	}

	maxItemsNum = nextPowerOf2(maxItemsNum);
	nHashLists = (maxItemsNum - 1) / tbl->hashListSize + 1;

    tbl->lowMask = nHashLists - 1;
    tbl->highMask = (nHashLists << 1) - 1;

	nSegs = (nHashLists - 1) / tbl->segmSize + 1;

	tbl->segments = (AHashSegment)_->memManager->alloc(tbl->segmsAmount * sizeof(HashSegment));
    
    /* Allocate initial segments */
	for (segP = tbl->segments; tbl->nSegs < nSegs; tbl->nSegs++, segP++)
	{
		segLLSize = tbl->segmSize * sizeof(HashList);
		*segP = (HashSegment)(AHashList)_->memManager->alloc(segLLSize);
	    memset(*segP, 0, segLLSize);
	}
    
    /* Our element consists of a header and data sp that the size 
     * is sum of the header's size and data's size */
    elemSize = ALIGN(sizeof(SHashItem)) + ALIGN(tbl->valLen);
	tbl->numItemsToAlloc = itemsNumToAlloc(elemSize);

	return tbl;
}

/* This function computes a number of items to allocate
 * when we need to extend a hashtable. For perfomance care 
 * we need to allocate a large enough piece of memory 
 * to avoid frequent malloc calls. 
 * We need to alloc at least 32 items. 
 * Suppose we are going to allocate (32 + k) hash items, where k >= 0
 * A total size of memory that will be allocated should be a power of two.
 */
uint itemsNumToAlloc(uint elemSize)
{  
    uint elemsToAlloc;
    uint allocMemSize;

    /* Suppose that our elemSize is at least 8 bits.
	 * Then supppose we allocate 32 items.
	 * In this case a memory to allocate is 32 * 8. 
	 * Now we need to check if our assumption was correct:
	 * Divide (32 * 8) on elemSize. If a number is more than 32
	 * that means that our elemSize is less than 8 and we return it.
	 * If the number is less than 32 we multiple allocMemSize for 2 
	 * and check again until we receive a number more or equal 32
	 */

	allocMemSize = 32 * 4;

    do
    {
	   allocMemSize <<= 1;
	   elemsToAlloc = allocMemSize / elemSize;
    } 
	while (elemsToAlloc < 32);

    return elemsToAlloc;
}